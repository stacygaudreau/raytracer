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
    live_gui,       // rendering for in the GUI in realtime
    render_only,    // rendering to eg: disk; the GUI is locked
    invalid = std::numeric_limits<uint8_t>::max()
};

/**
 * @brief Status of a rendering job
 */
enum class Status : uint8_t {
    waiting = 0,
    in_progress = 1,
    paused = 2,
    cancelled = 3,
    complete = 4,
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
 * @brief Describes a single rendering task request in the render queue.
 * @details Contains references to the 3d world, camera, and image targets to render.
 */
struct Job {
    Job(Camera& camera, World& world) : camera(camera),
                                        world(world),
                                        width(camera.getHSize()),
                                        height(camera.getVSize()) { }
    bool operator==(const Job& other) const {
        return this == &other;
    }
    Camera& camera;
    World& world;
    JobType type{ JobType::invalid };
    uint32_t width{}, height{};
    // progressive refinement pass block sizes in (NxN) pixels
    // eg: { 32, 16, 8, 1 } gives you 4 passes with 32px, 16px 8px and 1px resolutions
    std::vector<uint32_t> passes{ 1 };
    JobID id{ JobID_INVALID };
};


/**
 * @brief Render job state for the Scheduler
 */
struct JobState {
    explicit JobState(Job job) : job(std::move(job)) { }
    bool operator==(const JobState& other) const {
        return this == &other;
    }
    Job job;
    uint32_t nTilesRemain{ };
    Status status{ Status::invalid };
};


/**
 * @brief Rectangular region of an image to render
 */
struct Tile {
    explicit Tile(const std::shared_ptr<JobState> &state) : state(state) { }
    std::shared_ptr<JobState> state;
    JobID jobID{ JobID_INVALID };
    PKey priority{ PKey_MIN };
    uint32_t x0{}, y0{}, x1{}, y1{};
    uint32_t nPass{ 0 };
    uint32_t blockSize{ 1 };
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


    PRIVATE_IN_PRODUCTION
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
        const int64_t cx = width/2;
        const int64_t cy = height/2;
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
                                                 const uint32_t tileSize=32) {
        const auto& job = state->job;
        ASSERT(job.type != JobType::invalid, "job type must be specified before getting tiles");
        const auto W = job.width, H = job.height;
        std::vector<Tile> tiles;
        // one or more sets of tiles are created -- one for each progressive pass
        for (size_t nPass{}; nPass < job.passes.size(); ++nPass) {
            const auto blockSize = std::max(1u, job.passes.at(nPass));
            // tiles for a single pass
            for (uint32_t y{}; y < H; y += tileSize) {
                for (uint32_t x{}; x < W; x += tileSize) {
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
};



class Worker {
public:
    /**
     * @brief Single thread of rendering work, belonging to a RenderPool. Carries out rendering
     * RenderTiles assigned to it.
     */
    Worker() {}

private:
    std::shared_ptr<Tile> tile{ nullptr };
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