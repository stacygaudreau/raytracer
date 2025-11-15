/**
 *  
 *  Raytracer Lib - Render::JobFinalizer
 *
 *  @file job_finalizer.hpp
 *  @brief Consumes completed jobs from the Scheduler
 *  @author Stacy Gaudreau
 *  @date 2025.11.14
 *
 */


#pragma once

#include <thread>
#include <condition_variable>

#include "raytracer/logging/logging.hpp"
#include "raytracer/renderer/render_common.hpp"
#include "raytracer/utils/macros.hpp"
#include "raytracer/third_party/rigtorp/SPSCQueue.h"


namespace rt::Render {

class JobScheduler;

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Finalizes jobs which have completed in the scheduler
 * @details Takes care of eg: rendering files to disk, and calling back to
 * any programmer-supplied callbacks for eg: the GUI when a render job ends
 */
class JobFinalizer {
public:
    JobFinalizer() : queue(QUEUE_SIZE) {

    }
    ~JobFinalizer() {
        stop();
    }

    /**
     * @brief Push a job onto the finalization queue
     */
    void push(JobToFinalize&& job) {
        while (!queue.try_push(std::move(job))) {
            std::this_thread::yield();
            RENDER_WARN("Finalizer queue overrun, consider adjusting size");
        }
        signal.release();   // signal work avail.
    }
    /**
     * @brief Start the worker thread. Call stop to shutdown.
     */
    void start() {
        if (isRunning.load())
            return;
        RENDER_DEBUG("JobFinalizer starting");
        thread = std::make_unique<std::thread>([this](){ run(); });
    }
    /**
     * @brief Shutdown the Finalizer and its queue.
     */
    void stop() {
        if (!isRunning.exchange(false))
            return;
        RENDER_DEBUG("JobFinalizer stopping");
        signal.release();   // signal to stop
        if (thread->joinable()) {
            thread->join();
        }
    }

    /**
     * @brief Get a JobSummary report of a finalized job
     * @details This will return nothing until the job is actually processed from the queue
     */
    JobSummary* getSummary(JobID id) {
        std::scoped_lock lock{ m_finalized };
        if (auto it = finalized.find(id); it != finalized.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    void attachToScheduler(JobScheduler& s) {
        scheduler = &s;
    }


PRIVATE_IN_PRODUCTION
    void run() {
        RENDER_DEBUG("JobFinalizer running");
        isRunning = true;

        while (isRunning.load(std::memory_order_relaxed)) {
            finalizeAll();
            // sleep until signalled for jobs or shutdown
            using namespace std::literals::chrono_literals;
            signal.try_acquire_for(50ms);
        }
        RENDER_DEBUG("JobFinalizer stopped");
    }

    /**
     * @brief Flush the queue, finalizing all jobs in it
     */
    void finalizeAll() {
        while (!queue.empty()) {
            const auto& job = queue.front();
            RENDER_INFO("finalized job id: {}", job->summary.id);
            if (job->callback != nullptr) {
                job->callback(job->summary);
            }
            std::scoped_lock lock{ m_finalized };
            {
                if (finalized.contains(job->summary.id)) [[unlikely]] {
                    RENDER_ERROR("duplicate job summary with id {} in finalizer", job->summary.id);
                }
                finalized.emplace(job->summary.id, std::move(job->summary));
                if (scheduler != nullptr) {
                    // scheduler->erase
                } else [[unlikely]] {
                }
            }
            queue.pop();
        }
    }


PRIVATE_IN_PRODUCTION
    static constexpr size_t QUEUE_SIZE{ 1024 };
    rigtorp::SPSCQueue<JobToFinalize> queue;
    std::unique_ptr<std::thread> thread{ nullptr };
    std::atomic<bool> isRunning{ false };
    std::binary_semaphore signal{ 0 };  // binary fine since queue flushed for every call
    std::unordered_map<JobID, JobSummary> finalized;    // map of finalized job summaries
    std::mutex m_finalized; // protects summaries map
    JobScheduler* scheduler{ nullptr };

DELETE_COPY_AND_MOVE(JobFinalizer)
};
}