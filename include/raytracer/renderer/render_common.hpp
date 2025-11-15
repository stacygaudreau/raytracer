/**
 *  
 *  Raytracer lib
 *
 *  @file render_common.hpp
 *  @brief Common types and other components common to the Rendering namespace
 *  @author Stacy Gaudreau
 *  @date 2025.11.15
 *
 */


#pragma once

#include <cstdint>
#include <chrono>
#include <limits>
#include <atomic>
#include <functional>
#include "raytracer/environment/world.hpp"
#include "raytracer/environment/camera.hpp"


////////////////////////////////////////////////////////////////////////////////////////////////////
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
 * @brief Rendering mode
 */
enum class Mode : uint8_t {
    live_gui,    // rendering for in the GUI in realtime
    render_only, // rendering to eg: disk; the GUI is locked
    invalid = std::numeric_limits<uint8_t>::max()
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Target image output for rendering a job to
 * @details Contains details for where and how to render the image
 */
struct ImageTarget {
    ImageTarget(uint32_t width, uint32_t height) : buffer(width, height) { }

    Canvas buffer;                          // output in-memory image buffer
    std::string path{ "image_target.ppm" }; // target image path when rendering to disk
    bool operator==(const ImageTarget& other) const {
        return path == other.path
               && buffer.getWidth() == other.buffer.getWidth()
               && buffer.getHeight() == other.buffer.getHeight();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Records why a job has ended
 */
enum class JobEndReason : uint8_t {
    completed,
    cancelled,
    failed,
    invalid = std::numeric_limits<uint8_t>::max()
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Snapshot summary of an ended job, with some extra info for finalization
 */
struct JobSummary {
    explicit JobSummary(const ImageTarget& target) : target(target) { }
    JobID id{ JobID_INVALID };
    JobType type{ JobType::invalid };
    JobEndReason endReason{ JobEndReason::invalid };
    // output
    ImageTarget target;
    // render metrics
    uint32_t nTiles{};
    uint32_t nTilesComplete{};
    uint64_t nPixelsComplete{};
    uint32_t nPasses{};
    // timestamps
    std::chrono::steady_clock::time_point tSubmit{}, tStart{}, tComplete{};
};

/**
 * @brief Callback which occurs after a job ends
 */
using JobEndedCallback = std::function<void(const JobSummary&)>;

/**
 * @brief Ended job on its way to the Finalizer
 */
struct JobToFinalize {
    JobSummary summary;
    JobEndedCallback callback;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
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
    std::atomic<bool> isStarted{ false };   // flagged when rendering has begun
    std::atomic<bool> isCompleted{ false }; // true when completed (even if not 100% done)
    std::atomic<bool> isCancelled{ false }; // flag to stop queueing job tiles for render
    JobEndedCallback onJobEnd{ nullptr }; // callback fires on job end, after finalize
    // metrics
    uint32_t nTiles{};
    std::atomic<uint32_t> nTilesRemain{}; // tiles left in job
    std::atomic<uint32_t> nTilesComplete{}; // tiles actually rendered to completion
    std::atomic<uint64_t> nPixelsComplete{};
    // timestamps
    std::chrono::steady_clock::time_point tSubmit{}, tStart{}, tLastTile{}, tComplete{};
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

////////////////////////////////////////////////////////////////////////////////////////////////////


}
