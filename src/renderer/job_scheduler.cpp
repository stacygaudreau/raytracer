#include "raytracer/renderer/job_scheduler.hpp"
#include "raytracer/renderer/job_finalizer.hpp"

namespace rt::Render {

////////////////////////////////////////////////////////////////////////////////////////////////////
void JobScheduler::attachToFinalizer(JobFinalizer& f) {
    finalizer = &f;
    f.attachToScheduler(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void JobScheduler::setCompleteAndFinalize(const std::shared_ptr<JobState>& state) {
    if (state == nullptr) return;
    RENDER_DEBUG("completing job ID {}", state->job.id);
    state->isCompleted = true;
    const auto allTilesWereRendered = state->nTilesComplete.load(std::memory_order::relaxed) == state->nTiles;
    state->tComplete = allTilesWereRendered ? state->tLastTile : std::chrono::steady_clock::now();
    if (finalizer != nullptr) {
        finalizer->push({ makeSummary(state), state->onJobEnd });
    } else [[unlikely]] {
        RENDER_WARN("finalizer is null, job ID {} will not be finalized", state->job.id);
    }
}

}