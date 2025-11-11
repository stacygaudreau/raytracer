/**
 *  
 *  Raytracer Lib
 *
 *  @file renderer.hpp
 *  @brief Top-level rendering module
 *  @author Stacy Gaudreau
 *  @date 2025.11.02
 *
 */

#pragma once

#include <queue>

#include "raytracer/utils/macros.hpp"
#include "raytracer/renderer/canvas.hpp"
#include "raytracer/environment/camera.hpp"
#include "raytracer/environment/world.hpp"
#include "raytracer/renderer/colour.hpp"
#include "raytracer/logging/logging.hpp"
#include "raytracer/types.hpp"
#include <vector>
#include <cmath>
#include <condition_variable>
#include <thread>

namespace rt::Render {
/**
 * @brief Type of rendering job to run
 */
enum class JobType : uint8_t {
    realtime = 0,   // high priority realtime GUI bitmaps
    background = 1, // disk cached thumbnail generation etc.
    offline = 2,    // print quality image rendering to disk
    invalid = std::numeric_limits<uint8_t>::max()
};

inline constexpr uint64_t type_to_priority(JobType type) noexcept {
    return static_cast<uint64_t>(type);
}

/**
 * @brief Rendering mode
 */
enum class Mode : uint8_t {
    live_gui,    // rendering for in the GUI in realtime
    render_only, // rendering to eg: disk; the GUI is locked
    invalid = std::numeric_limits<uint8_t>::max()
};

/** @brief Numerical identifier for render job */
using JobID = uint64_t;
constexpr auto JobID_INVALID = std::numeric_limits<JobID>::max();

/**
 * @brief Priority key ranks tile priority in the render queue
 * @details Bit packing: [JobType:8 | n_pass:8 | dist:16 | reserved:32]
 */
using PKey = uint64_t;
constexpr auto PKey_MIN = std::numeric_limits<uint64_t>::max();

/**
 * @brief Target image output for rendering a job to
 * @details Contains details for where and how to render the image
 */
struct ImageTarget {
    ImageTarget(uint32_t width, uint32_t height) : buffer(width, height) { }
    Canvas buffer;  // output in-memory image buffer
    std::string path{ "image_target.ppm" }; // target image path when rendering to disk
};


/**
 * @brief Describes a single rendering task request in the render queue.
 * @details Contains references to the 3d world, camera, and image targets to render.
 */
struct Job {
    Job(Camera& camera, World& world, JobType type) : camera(camera),
                                                      world(world),
                                                      type(type),
                                                      width(camera.getHSize()),
                                                      height(camera.getVSize()),
                                                      target(camera.getHSize(),
                                                          camera.getVSize()) {
    }

    bool operator==(const Job& other) const {
        return id == other.id;
    }

    Camera& camera;
    World& world;
    JobType type;
    uint32_t width{ }, height{ };
    ImageTarget target;
    // progressive refinement pass block sizes in (NxN) pixels
    // eg: { 32, 16, 8, 1 } gives you 4 passes with 32px, 16px 8px and 1px resolutions
    std::vector<uint32_t> passes{ 1 };
    JobID id{ JobID_INVALID };
};


/**
 * @brief Render job state for the Scheduler
 */
struct JobState {
    explicit JobState(Job job) : job(std::move(job)) {
    }

    bool operator==(const JobState& other) const {
        return job == other.job;
    }

    Job job;
    std::atomic<uint32_t> nTilesRemain{ };
    std::atomic<bool> isStarted{ false };   // flagged when rendering has begun
    std::atomic<bool> isFinalized{ false }; // true when the job is ending (whether 100% done or not)
    std::atomic<bool> isCancelled{ false }; // flag to stop queueing job work for render
};


/**
 * @brief Rectangular region of an image to render
 */
struct Tile {
    explicit Tile(const std::shared_ptr<JobState>& state) : state(state) {
    }

    std::shared_ptr<JobState> state;
    JobID jobID{ JobID_INVALID };
    PKey priority{ PKey_MIN };
    uint32_t x0{ }, y0{ }, x1{ }, y1{ };
    uint32_t nPass{ 0 };
    uint32_t blockSize{ 1 };

    bool operator==(const Tile& other) const {
        return x0 == other.x0
               && x1 == other.x1
               && y0 == other.y0
               && y1 == other.y1
               && jobID == other.jobID;
    }
};

/**
 * @brief Comparison functor helps sort tiles by max priority (lesser pkey value)
 */
struct LesserPKeyValue {
    bool operator()(const Tile& A, const Tile& B) const {
        return A.priority > B.priority;
    }
};

/*
 *  Render::JobScheduler
 */
class JobScheduler {
public:
    /**
     * @brief Manages incoming and in-progress RenderJobs. Handles dispatching and ordering
     * based on priority and attributes.
     */
    JobScheduler() {
    }

    ~JobScheduler() {
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
        state->nTilesRemain = static_cast<uint32_t>(jobTiles.size());
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
            if (it == jobs.end()
                || it->second->isCancelled.load(std::memory_order_relaxed)) {
                // decrement remaining, finalize job at end
                if (t.state != nullptr && t.state->nTilesRemain.fetch_sub(1) <= 1) {
                    RENDER_DEBUG("finalizing job ID {}", t.jobID);
                    t.state->isFinalized = true;
                    jobs.erase(t.jobID);
                }
                continue;
            }
            return t;
        }
        return std::nullopt;
    }
    /**
     * @brief Mark a given tile completed.
     * @details Called by Workers to set a tile complete. This will deadlock if you
     * call it from the same thread as getNextTile() when the queue is empty.
     */
    void setTileComplete(const Tile& t) {
        auto state = t.state;
        if (state == nullptr)
            return;
        if (state->nTilesRemain.fetch_sub(1) <= 1) {
            t.state->isFinalized = true;
            jobs.erase(t.jobID);
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
     * @brief Get the next Job number in the sequence
     */
    JobID getNextJobID() {
        std::scoped_lock{ m_jobID };
        return ++jobID;
    }

PRIVATE_IN_PRODUCTION
    TileQueue tiles;
    std::unordered_map<JobID, std::shared_ptr<JobState>> jobs;
    std::mutex m_tiles;
    std::condition_variable cv_tiles; // signal for tiles queue status
    bool inShutdown{ false };
    JobID jobID{ JobID_INVALID };
    mutable std::mutex m_jobID;
    Mode mode{ Mode::live_gui };
};


class Worker {
public:
    /**
     * @brief Single thread of rendering work, belonging to a WorkerPool. Carries out rendering
     * Tiles assigned to it.
     */
    Worker(uint32_t id, JobScheduler& scheduler) : id(id), scheduler(scheduler) {
    }

    ~Worker() {
        stop();
    }

    void start() {
        if (isRunning.exchange(true))
            return;
        thread = std::make_unique<std::thread>([this]() { run(); });
        RENDER_DEBUG("<{}> worker started", id);
    }

    void stop() {
        if (!isRunning.exchange(false))
            return;
        if (thread != nullptr && thread->joinable())
            thread->join();
        RENDER_DEBUG("<{}> worker stopped", id);
    }

PRIVATE_IN_PRODUCTION
    void run() {
        RENDER_DEBUG("<{}> worker running", id);
        while (isRunning.load(std::memory_order_relaxed)) {
            auto t = scheduler.getNextTile();
            if (!t) {
                RENDER_DEBUG("<{}> worker shutdown signal received", id);
                break;
            }
            if (t->state->isCancelled.load(std::memory_order_relaxed)) {
                scheduler.setTileComplete(*t);
                continue;
            }
            // todo: RENDER TILE
            scheduler.setTileComplete(*t);
        }
    }

    uint32_t id;
    JobScheduler& scheduler;
    std::unique_ptr<std::thread> thread{ nullptr };
    std::atomic<bool> isRunning{ false };
    // todo: render output buffer target
};


class RenderEngine {
public:
    /**
     * @brief The top-level rendering module. This contains all of the sub-components
     * needed for rendering various types of image output.
     * @details There should generally only be one of these per application.
     */
    RenderEngine() {
        Log::init();
        RENDER_TRACE("created rendering engine");
    }

    ~RenderEngine() {
        RENDER_TRACE("rendering engine destroyed");
    }

    PRIVATE_IN_PRODUCTION
    DELETE_COPY_AND_MOVE(RenderEngine)
};
}
