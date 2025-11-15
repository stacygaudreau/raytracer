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
#include "raytracer/renderer/job_scheduler.hpp"
#include "raytracer/logging/logging.hpp"
#include "raytracer/types.hpp"
#include <vector>
#include <cmath>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <semaphore>

namespace rt::Render {

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
