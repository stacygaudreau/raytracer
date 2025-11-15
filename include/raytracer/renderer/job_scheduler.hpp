/**
 *  
 *  Raytracer Lib - Render::JobScheduler
 *
 *  @file job_scheduler.hpp
 *  @brief Manages and schedules a priority queue of rendering Jobs
 *  @author Stacy Gaudreau
 *  @date 2025.11.14
 *
 */


#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>
#include <atomic>

#include "raytracer/logging/logging.hpp"
#include "raytracer/renderer/render_common.hpp"
#include "raytracer/common/macros.hpp"
#include "raytracer/third_party/rigtorp/SPSCQueue.h"


namespace rt::Render {

class JobFinalizer;

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Comparison functor helps sort tiles by max priority (lesser pkey value)
 */
struct LesserPKeyValue {
    bool operator()(const Tile& A, const Tile& B) const {
        return A.priority > B.priority;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief This is the brain of the render system. Manages incoming and in-progress Render::Jobs.
 * @details Handles dispatching and ordering of jobs based on priority and attributes. A pool of
 * workers obtain and render fragments (tiles) of a given job from the Scheduler.
 */
class JobScheduler {
public:

    JobScheduler() {
    }

    ~JobScheduler() {
        shutdown();
    }

    /**
     * @brief Submit a job to the queue for rendering. The job is consumed
     * and no longer valid once submitted.
     * @param job Job to render.
     * @return ID of the job which can be used to track it.
     */
    JobID submit(Job job) {
        job.id = getNextJobID();
        auto state = std::make_shared<JobState>(job);
        auto jobTiles = getTilesForJobState(state);
        state->nTiles = static_cast<uint32_t>(jobTiles.size());
        state->nTilesRemain = state->nTiles;
        state->tSubmit = std::chrono::steady_clock::now();
        // queue is loaded with the job's tiles
        {
            std::scoped_lock lock{ m_tiles };
            jobs.emplace(state->job.id, state);
            for (auto& t: jobTiles) {
                tiles.push(std::move(t));
            }
        }
        state->isStarted = true;
        // notify waiting workers
        // TODO: optimize to notify min(n_pool_size, n_tiles_loaded) times
        //  => may reduce job to work completion latency and reduce context switching
        cv_tiles.notify_all();
        return state->job.id;
    }
    /**
     * @brief Cancel a job with the given ID
     */
    void cancel(JobID id) {
        std::scoped_lock lock{ m_tiles };
        if (auto it = jobs.find(id); it != jobs.end()) {
            it->second->isCancelled = true;
        }
    }

    /**
     * @brief Connect to a JobScheduler. This must be done once
     * @details Creates a bidirectional connection with JobScheduler
     */
    void attachToFinalizer(JobFinalizer& f);
    /**
     * @brief Get the next (highest priority) tile in the render queue to work on
     */
    std::optional<Tile> getNextTile() {
        std::unique_lock lock{ m_tiles };
        cv_tiles.wait(lock, [&] { return !tiles.empty() || inShutdown; });
        if (inShutdown) {
            RENDER_DEBUG("shutdown signal received");
            return std::nullopt;
        }
        // discard any tiles for inactive or cancelled jobs
        while (!tiles.empty()) {
            auto t = std::move(const_cast<Tile&>(tiles.top()));
            tiles.pop();
            auto it = jobs.find(t.jobID);
            const bool isInvalid = it == jobs.end()
                                   || it->second->isCancelled.load(std::memory_order_relaxed);
            if (isInvalid) {
                // on last tile in job, send to Finalizer
                if (t.state != nullptr && t.state->nTilesRemain.fetch_sub(1) <= 1) {
                    setCompleteAndFinalize(t.state);
                }
                continue;
            }
            return t;
        }
        return std::nullopt;
    }
    /**
     * @brief Mark a given tile completely rendered.
     * @details Called by Workers to set a tile completely rendered. This increments
     * nTilesComplete and will complete the job if needed.
     */
    void setTileComplete(const Tile& t) {
        auto state = t.state;
        if (state == nullptr) return;
        state->tLastTile = std::chrono::steady_clock::now();
        ++state->nTilesComplete;
        // if it's the final tile, we send to finalizer with the most recent
        //  tile completion timestamp for completion time
        if (state->nTilesRemain.fetch_sub(1) <= 1) {
            setCompleteAndFinalize(state);
        }
    }
    /**
     * @brief Request all threads waiting on tiles to shutdown
     */
    void shutdown() {
        {
            std::scoped_lock lock{ m_tiles };
            inShutdown = true;
        }
        cv_tiles.notify_all();
    }
    /**
     * @brief Get the state (if found) for a given job ID
     */
    std::shared_ptr<JobState> getJobState(JobID id) {
        std::scoped_lock lock{ m_tiles };
        if (auto it = jobs.find(id); it != jobs.end()) {
            return it->second;
        }
        return nullptr;
    }
    /**
     * @brief Delete job with given ID from the JobState register
     */
    void eraseJobState(JobID id) {
        std::scoped_lock lock{ m_tiles };
        jobs.erase(id);
    }
    /**
     * @brief Construct a summary snapshot of a job
     */
    static JobSummary makeSummary(const std::shared_ptr<JobState>& state) {
        ASSERT(state != nullptr, "job state is null, cannot make job summary");
        JobSummary s{ state->job.target };
        s.id = state->job.id;
        s.type = state->job.type;
        s.target = state->job.target;
        s.nTiles = state->nTiles;
        s.nTilesComplete = state->nTilesComplete.load(std::memory_order::relaxed);
        s.nPixelsComplete = state->nPixelsComplete.load(std::memory_order::relaxed);
        s.nPasses = std::max(static_cast<uint32_t>(state->job.passes.size()), 1u);
        s.tSubmit = state->tSubmit;
        s.tStart = state->tStart;
        s.tComplete = state->tComplete;
        if (state->isCompleted.load(std::memory_order::relaxed)) {
            s.endReason = JobEndReason::completed;
        } else if (state->isCancelled.load(std::memory_order::relaxed)) {
            s.endReason = JobEndReason::cancelled;
        } else {
            s.endReason = JobEndReason::failed;
        }
        return s;
    }

PRIVATE_IN_PRODUCTION
    /**
     * @brief Maximum render priority tile queue, sorted by PKey value
     */
    using TileQueue = std::priority_queue<Tile, std::vector<Tile>, LesserPKeyValue>;
    /**
     * @brief Make a 64bit scheduler priority key for a given tile render configuration
     * @details Priority is determined by JobType, progressive pass number, and tile distance to
     * center of image. MAX priority is zero, ie: lower number => higher priority
     * @param type render job type
     * @param nPass progressive rendering pass number
     * @param tile_cx tile center x
     * @param tile_cy tile center y
     * @param width total image width
     * @param height total image height
     * @return priority ranked key for tile
     */
    static PKey getPriorityKeyForTile(JobType type, uint32_t nPass, uint32_t tile_cx, uint32_t tile_cy,
                                      uint32_t width, uint32_t height) {
        ASSERT(type != JobType::invalid, "invalid job type for tile priority");
        const auto p_type = type_to_priority(type) << 56;
        const auto p_pass = static_cast<uint64_t>(nPass & 0xFF) << 48;
        // fast manhattan distance from centre of image
        const int64_t cx = width / 2;
        const int64_t cy = height / 2;
        const auto dist = static_cast<uint64_t>(std::abs(static_cast<int64_t>(tile_cx) - cx)
                                                + std::abs(static_cast<int64_t>(tile_cy) - cy));
        // normalise to image size and 16bit scale
        const auto max_dist = static_cast<uint64_t>(cx + cy);
        const auto d_norm = static_cast<double>(dist) / max_dist;
        const auto p_dist = static_cast<uint64_t>(d_norm * 0xFFFF) << 32;
        // final priority is [type | n_pass | distance]
        return p_type | p_pass | p_dist;
    }
    /**
     * @brief Break up a Job into a sequence of prioritized RenderTiles.
     */
    static std::vector<Tile> getTilesForJobState(const std::shared_ptr<JobState>& state,
                                                 const uint32_t tileSize = 32) {
        const auto& job = state->job;
        ASSERT(job.type != JobType::invalid, "job type must be specified before getting tiles");
        const auto W = job.width, H = job.height;
        std::vector<Tile> tiles;
        // one or more sets of tiles are created -- one for each progressive pass
        for (size_t nPass{ }; nPass < job.passes.size(); ++nPass) {
            const auto blockSize = std::max(1u, job.passes.at(nPass));
            // tiles for a single pass
            for (uint32_t y{ }; y < H; y += tileSize) {
                for (uint32_t x{ }; x < W; x += tileSize) {
                    Tile t{ state };
                    t.jobID = state->job.id;
                    t.nPass = nPass;
                    t.blockSize = blockSize;
                    // x0 y0 are inclusive
                    t.x0 = x;
                    t.y0 = y;
                    // x1 y1 are exclusive
                    t.x1 = std::min(W, x + tileSize);
                    t.y1 = std::min(H, y + tileSize);
                    // priority in queue
                    const auto cx = (t.x1 + t.x0) / 2;
                    const auto cy = (t.y1 + t.y0) / 2;
                    t.priority = getPriorityKeyForTile(job.type, nPass, cx, cy, W, H);
                    tiles.emplace_back(std::move(t));
                }
            }
        }
        return tiles;
    }

    /**
     * @brief Helper to complete a job and send it to the finalizer
     * @details NOT thread safe! Intended to be called within a thread-safe block.
     */
    void setCompleteAndFinalize(const std::shared_ptr<JobState>& state);

    /**
     * @brief Get the next Job number in the sequence
     */
    JobID getNextJobID() {
        std::scoped_lock{ m_jobID };
        return ++jobID;
    }

PRIVATE_IN_PRODUCTION
    TileQueue tiles;
    std::unordered_map<JobID, std::shared_ptr<JobState>> jobs;  // jobs in progress
    std::mutex m_tiles;
    std::condition_variable cv_tiles; // signal for tiles queue status
    bool inShutdown{ false };
    JobID jobID{ JobID_INVALID };
    mutable std::mutex m_jobID;
    Mode mode{ Mode::live_gui };
    JobFinalizer* finalizer{ nullptr };

    DELETE_COPY_AND_MOVE(JobScheduler)
};

}