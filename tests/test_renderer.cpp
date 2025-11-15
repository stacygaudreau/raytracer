#include "gtest/gtest.h"
#include "raytracer/renderer/renderer.hpp"
#include "raytracer/environment/camera.hpp"
#include "raytracer/renderer/canvas.hpp"
#include "raytracer/shapes/sphere.hpp"
#include "raytracer/logging/logging.hpp"
#include "raytracer/renderer/job_scheduler.hpp"
#include "raytracer/renderer/job_finalizer.hpp"
#include <chrono>

using namespace rt;
using namespace rt::Render;
using namespace std::literals::chrono_literals;


/*
 *  RenderEngine
 */
class RenderEngineTests: public ::testing::Test
{
protected:
    World world{};
    PointLight light{Point{-10, 10, -10}, Colour{1, 1, 1}};
    Material m1{{0.8, 1.0, 0.6}, 0.1, 0.7, 0.2};
    Sphere s1{}, s2{};

    Camera camera{ 256, 256, HALF_PI };
    std::unique_ptr<RenderEngine> renderer{ nullptr };

    void SetUp() override {
        s1.setMaterial(m1);
        s2.setTransform(Transform::scale(.5, .5, .5));
        world.addLight(PointLight{Point{-10, 10, -10}, Colour{1, 1, 1}});
        world.addShape(&s1);
        world.addShape(&s2);
        // camera orientation
        auto from = Point{ 0., 0., -5. };
        auto to = Point{};
        auto up = Vector{ 0., 1., 0. };
        camera.setTransform(Transform::viewTransform(from, to, up));
        renderer = std::make_unique<RenderEngine>();
    }
};

TEST_F(RenderEngineTests, ConstructedWithBasicProperties) {
    // a Renderer instance is constructed with basic properties
    EXPECT_NE(renderer, nullptr);
}


/*
 *  RenderJob
 */
class RenderJobTests: public RenderEngineTests {
protected:
    std::unique_ptr<Job> job{ nullptr };
    void SetUp() override {
        RenderEngineTests::SetUp();
        job = std::make_unique<Job>(camera, world, JobType::invalid);
    }
};

TEST_F(RenderJobTests, ConstructedWithProperties) {
    EXPECT_EQ(job->id, JobID_INVALID);
    EXPECT_EQ(job->type, JobType::invalid);
    EXPECT_EQ(job->passes.size(), 1);
    EXPECT_EQ(job->passes.at(0), 1);
    EXPECT_EQ(job->width, camera.getHSize());
    EXPECT_EQ(job->height, camera.getVSize());
    EXPECT_EQ(job->target.buffer.getWidth(), camera.getHSize());
    EXPECT_EQ(job->target.buffer.getHeight(), camera.getVSize());
}


/*
 *  RenderJobState
 */
class RenderJobStateTests: public RenderJobTests {
protected:
    std::unique_ptr<JobState> state{ nullptr };
    static constexpr JobID id{ 12345 };
    void SetUp() override {
        RenderJobTests::SetUp();
        state = std::make_unique<JobState>(std::move(*job));
        state->job.id = id;
    }
};

TEST_F(RenderJobStateTests, ConstructedWithProperties) {
    EXPECT_NE(state, nullptr);
    EXPECT_EQ(state->job.id, id);
    EXPECT_EQ(state->nTilesRemain.load(), 0);
    EXPECT_FALSE(state->isStarted.load());
    EXPECT_FALSE(state->isCompleted.load());
    EXPECT_FALSE(state->isCancelled.load());
    EXPECT_EQ(state->onJobEnd, nullptr);
}


/*
 *  RenderScheduler
 */
class RenderJobSchedulerTests: public RenderEngineTests {
protected:
    std::unique_ptr<JobScheduler> sched{ nullptr };
    Camera cam{ 256, 256, HALF_PI };

    void SetUp() override {
        RenderEngineTests::SetUp();
        auto from = Point{ 0., 0., -5. };
        auto to = Point{};
        auto up = Vector{ 0., 1., 0. };
        cam.setTransform(Transform::viewTransform(from, to, up));
        sched = std::make_unique<JobScheduler>();
    }
};

TEST_F(RenderJobSchedulerTests, DefaultProperties) {
    ASSERT_NE(sched, nullptr);
    EXPECT_EQ(sched->finalizer, nullptr);
}

TEST_F(RenderJobSchedulerTests, GetEvenTilesForSinglePassSquareViewportJob) {
    // a single pass job is broken up into a set of tiles
    //  from a camera with a perfectly square viewport with EVEN tile size
    cam.setHSize(256); cam.setVSize(256);
    Job job{ cam, world, JobType::background };
    job.id = 12345;
    auto state = std::make_shared<JobState>(job);
    auto tiles = sched->getTilesForJobState(state, 32);
    EXPECT_EQ(tiles.size(), 64);
    // verify corner tiles x,y
    auto t1 = tiles.at(0);
    EXPECT_EQ(t1.x0, 0);
    EXPECT_EQ(t1.x1, 32);
    EXPECT_EQ(t1.y0, 0);
    EXPECT_EQ(t1.y1, 32);
    auto t2 = tiles.at(7);
    EXPECT_EQ(t2.x0, 256-32);
    EXPECT_EQ(t2.x1, 256);
    EXPECT_EQ(t2.y0, 0);
    EXPECT_EQ(t2.y1, 32);
    auto t3 = tiles.at(64-8);
    EXPECT_EQ(t3.x0, 0);
    EXPECT_EQ(t3.x1, 32);
    EXPECT_EQ(t3.y0, 256-32);
    EXPECT_EQ(t3.y1, 256);
    auto t4 = tiles.at(63);
    EXPECT_EQ(t4.x0, 256-32);
    EXPECT_EQ(t4.x1, 256);
    EXPECT_EQ(t4.y0, 256-32);
    EXPECT_EQ(t4.y1, 256);
    // verify other tile attributes set by fn
    for (const auto& t : tiles) {
        EXPECT_EQ(t.state, state);
        EXPECT_EQ(t.jobID, job.id);
        EXPECT_EQ(t.nPass, 0);
    }
}

TEST_F(RenderJobSchedulerTests, GetSingleTileWhenImageIsSmall) {
    // tileSize is larger than the actual image
    cam.setHSize(31); cam.setVSize(31);
    Job job{ cam, world, JobType::background };
    auto state = std::make_shared<JobState>(job);
    auto tiles = sched->getTilesForJobState(state, 32);
    EXPECT_EQ(tiles.size(), 1);
    // tileSize is exactly the image size
    cam.setHSize(32); cam.setVSize(32);
    Job job2{ cam, world, JobType::background };
    auto state2 = std::make_shared<JobState>(job2);
    tiles = sched->getTilesForJobState(state2, 32);
    EXPECT_EQ(tiles.size(), 1);
    // tileSize is larger than the actual image on one axis
    cam.setHSize(33); cam.setVSize(32);
    Job job3{ cam, world, JobType::background };
    auto state3 = std::make_shared<JobState>(job3);
    tiles = sched->getTilesForJobState(state3, 32);
    EXPECT_EQ(tiles.size(), 2);
    // t3.0
    auto t3_0 = tiles.at(0);
    EXPECT_EQ(t3_0.x0, 0);
    EXPECT_EQ(t3_0.x1, 32);
    EXPECT_EQ(t3_0.y0, 0);
    EXPECT_EQ(t3_0.y1, 32);
    // t3.1
    auto t3_1 = tiles.at(1);
    EXPECT_EQ(t3_1.x0, 32);
    EXPECT_EQ(t3_1.x1, 33);
    EXPECT_EQ(t3_1.y0, 0);
    EXPECT_EQ(t3_1.y1, 32);
}

TEST_F(RenderJobSchedulerTests, GetTilesForSinglePassOddTileSizeRect) {
    // a single pass job with a rectangular camera viewport is broken up into a set
    //   of odd tiles, resulting in some oddly sized tiles at the edges
    cam.setHSize(62); cam.setVSize(32);
    constexpr uint32_t tileSize{ 15 };
    Job job{ cam, world, JobType::background };
    auto state = std::make_shared<JobState>(job);
    auto tiles = sched->getTilesForJobState(state, tileSize);
    // tile matrix would be 5x3
    EXPECT_EQ(tiles.size(), 15);
    // verify right and a bottom edge tile
    auto t1 = tiles.at(4);
    EXPECT_EQ(t1.x0, 60);
    EXPECT_EQ(t1.x1, 62);
    EXPECT_EQ(t1.y0, 0);
    EXPECT_EQ(t1.y1, 15);
    auto t2 = tiles.at(10); // bottom left
    EXPECT_EQ(t2.x0, 0);
    EXPECT_EQ(t2.x1, 15);
    EXPECT_EQ(t2.y0, 30);
    EXPECT_EQ(t2.y1, 32);
    // verify n_tile_pixels is same as target image WxH
    const auto imagePx = cam.getHSize() * cam.getVSize();
    uint32_t pxTotal{ };
    for (const auto& t: tiles) {
        const auto dx = t.x1 - t.x0;
        const auto dy = t.y1 - t.y0;
        pxTotal += (dx * dy);
    }
    EXPECT_EQ(pxTotal, imagePx);
}

TEST_F(RenderJobSchedulerTests, GetPriorityKey_OfflineCenter1st) {
    // tile in center of image, background job type, 1st pass
    auto key = sched->getPriorityKeyForTile(JobType::offline, 0, 128, 128, 256, 256);
    EXPECT_NE(key, PKey_MIN);
    // 0x02000000 (don't care about 32lsb)
    EXPECT_EQ(key, 0x0200000000000000) << std::hex << key;
}

TEST_F(RenderJobSchedulerTests, GetPriorityKey_BackgroundMidCorners5th) {
    // tile half way mid-corner in image, background job type, 5th pass
    auto key = sched->getPriorityKeyForTile(JobType::background, 4, 64, 64, 256, 256);
    EXPECT_NE(key, PKey_MIN);
    // 0x01047FFF (don't care about 32lsb)
    EXPECT_EQ(key, 0x01047FFF00000000) << std::hex << key;
}

TEST_F(RenderJobSchedulerTests, GetPriorityKey_RealtimeMaxDist65th) {
    // max tile distance (corner), realtime job type, 65th pass
    auto key = sched->getPriorityKeyForTile(JobType::realtime, 65, 0, 0, 256, 256);
    EXPECT_NE(key, PKey_MIN);
    // 0x0041FFFF (don't care about 32lsb)
    EXPECT_EQ(key, 0x0041FFFF00000000) << std::hex << key;
}

TEST_F(RenderJobSchedulerTests, GetPriorityKey_OfflineMid3rd) {
    // top center tile, offline job type, 3rd pass
    auto key = sched->getPriorityKeyForTile(JobType::offline, 3, 128, 0, 256, 256);
    EXPECT_NE(key, PKey_MIN);
    // 0x02037FFF (don't care about 32lsb)
    EXPECT_EQ(key, 0x02037FFF00000000) << std::hex << key;
}

TEST_F(RenderJobSchedulerTests, GetTilesForMultiPass) {
    // a job with more than one progressive rendering pass
    //  is broken into an appropriate set of tiles
    cam.setHSize(128); cam.setVSize(128);
    Job job{ cam, world, JobType::background };
    job.id = 12345;
    job.passes = { 16, 8, 4, 1 };
    auto state = std::make_shared<JobState>(job);
    auto tiles = sched->getTilesForJobState(state, 32);
    EXPECT_EQ(tiles.size(), 64);
    // a tile from the second pass is correctly tagged
    auto t = tiles.at(16);
    EXPECT_EQ(t.nPass, 1);
    EXPECT_EQ(t.blockSize, 8);
}

TEST_F(RenderJobSchedulerTests, SchedulerAssignsAllTileProperties) {
    // priority keys and other properties are indeed
    //  assigned to tiles by the scheduler
    cam.setHSize(128); cam.setVSize(128);
    Job job{ cam, world, JobType::offline };
    job.id = 12345;
    job.passes = { 16, 8, 4, 1 };
    auto state = std::make_shared<JobState>(job);
    auto tiles = sched->getTilesForJobState(state, 32);
    EXPECT_EQ(tiles.size(), 64);
    // verify a tile has sane values for each of its properties
    auto t = tiles.at(32);
    EXPECT_EQ(t.state->job.type, job.type);
    EXPECT_EQ(t.nPass, 2);  // 3rd progressive pass
    EXPECT_EQ(t.blockSize, 4);
    EXPECT_NE(t.jobID, JobID_INVALID);
    EXPECT_EQ(t.state, state);
    EXPECT_NE(t.priority, PKey_MIN);
}

TEST_F(RenderJobSchedulerTests, QueueTopIsLowestPKeyValue) {
    // the tile queue is sorted by min priority PKey
    //  (smaller PKey => higher actual priority)
    auto job = Job{ camera, world, JobType::offline };
    auto state = std::make_shared<JobState>(job);
    auto t_p2 = Tile{ state }; // lowest
    t_p2.priority = PKey_MIN - 100;
    auto t_p1 = Tile{ state }; // higher
    t_p1.priority = t_p2.priority - 100;
    auto t_p0 = Tile{ state }; // highest
    t_p0.priority = 0;
    // add to queue in wrong priority
    sched->tiles.emplace(t_p1);
    sched->tiles.emplace(t_p0);
    sched->tiles.emplace(t_p2);
    // order should be p0, p1, p2 when popping
    EXPECT_EQ(sched->tiles.top(), t_p0);
    sched->tiles.pop();
    EXPECT_EQ(sched->tiles.top(), t_p1);
    sched->tiles.pop();
    EXPECT_EQ(sched->tiles.top(), t_p2);
    sched->tiles.pop();
    EXPECT_TRUE(sched->tiles.empty());
}

TEST_F(RenderJobSchedulerTests, GetNextJobID) {
    //  the next job ID is returned from the scheduler
    EXPECT_EQ(sched->jobID, JobID_INVALID);
    auto next = sched->getNextJobID();
    EXPECT_EQ(next, 0);
    next = sched->getNextJobID();
    EXPECT_EQ(next, 1);
}

TEST_F(RenderJobSchedulerTests, JobIsSubmitted) {
    // a single job is added to the queue and all details
    //  are verified
    cam.setHSize(64);
    cam.setVSize(64);
    sched->jobID = 9000;
    Job job{ cam, world, JobType::offline };
    auto t0 = std::chrono::steady_clock::now();
    auto id = sched->submit(job);
    // the job was given an appropriate ID
    EXPECT_EQ(id, 9001);
    EXPECT_EQ(sched->jobID, 9001);
    // each expected tile is present in the queue
    Job job_dupe{ cam, world, JobType::offline };
    job_dupe.id = 9001;
    auto state_req = std::make_shared<JobState>(job_dupe);
    auto ts_req = sched->getTilesForJobState(state_req);
    std::vector<Tile> ts;
    while (!sched->tiles.empty()) {
        ts.emplace_back(sched->tiles.top());
        sched->tiles.pop();
    }
    // verify each tile was actually recv'd from the queue
    for (auto& t: ts_req) {
        EXPECT_TRUE(std::ranges::contains(ts, t));
    }
    // state for the job is maintained on the map
    //  and correct properties are set on the state
    auto state = std::shared_ptr<JobState>{ nullptr };
    auto it = sched->jobs.find(9001);
    if (it != sched->jobs.end()) {
        state = it->second;
    }
    ASSERT_NE(state, nullptr);
    EXPECT_EQ(state->nTiles, ts_req.size());
    EXPECT_EQ(state->nTilesRemain.load(), ts_req.size());
    EXPECT_GT(state->tSubmit, t0);
    EXPECT_TRUE(state->isStarted.load());
}

TEST_F(RenderJobSchedulerTests, IgnoreOfflineJobsInLiveGUIMode) {
    // submitting an offline type render job is ignored when
    //  the scheduler is in Live GUI rendering mode
    EXPECT_TRUE(false);
}

TEST_F(RenderJobSchedulerTests, GetNoTileWhenEmpty) {
    // no tile is returned, when the queue is empty
    auto th = std::jthread{
        [&] {
            std::this_thread::sleep_for(15ms);
            sched->shutdown();
        }
    };
    auto t = sched->getNextTile();
    EXPECT_FALSE(t);
}

TEST_F(RenderJobSchedulerTests, GetNextPriorityTile) {
    // the getNextTile method returns the highest priority
    //  tile in the queue
    // a single job is added to the queue and all details
    //  are verified
    EXPECT_EQ(sched->jobs.size(), 0);
    cam.setHSize(96);
    cam.setVSize(32); // 3x1 tiles
    Job job{ cam, world, JobType::offline };
    job.id = 0;
    auto state = std::make_shared<JobState>(job);
    auto tiles = sched->getTilesForJobState(state);
    const auto id = sched->submit(job);
    EXPECT_EQ(sched->jobs.size(), 1);
    // middle tile should be highest priority (centre)
    auto t_received = sched->getNextTile();
    EXPECT_EQ(t_received, tiles.at(1));
}

TEST_F(RenderJobSchedulerTests, GetJobState) {
    // the correct state is returned (or none) from the job
    //  state register
    cam.setHSize(256);
    cam.setVSize(256);
    const auto id1 = sched->submit({
        cam, world, JobType::realtime
    });
    const auto id2 = sched->submit({
        cam, world, JobType::realtime
    });
    // id1
    auto s1 = sched->getJobState(id1);
    auto s2 = sched->getJobState(id2);
    ASSERT_NE(nullptr, s1);
    ASSERT_NE(nullptr, s2);
    EXPECT_EQ(id1, sched->getJobState(id1)->job.id);
    EXPECT_EQ(id2, sched->getJobState(id2)->job.id);
    // not found
    EXPECT_EQ(sched->getJobState(9001), nullptr);
}

TEST_F(RenderJobSchedulerTests, SetTileUpdatesCounts) {
    // calling .setTileComplete() increments and decrements the
    //  proper counters on JobState
    cam.setHSize(64);
    cam.setVSize(64);
    const auto id = sched->submit({
        cam, world, JobType::offline
    });
    auto t0 = std::chrono::steady_clock::now();
    auto s = sched->getJobState(id);
    auto t = sched->getNextTile();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->nTilesRemain.load(), 4);
    // completing the tile decrements remaining
    sched->setTileComplete(t.value());
    EXPECT_EQ(s->nTilesRemain.load(), 3);
    // it also increments the actual rendered count
    EXPECT_EQ(s->nTilesComplete.load(), 1);
    // the time of last tile render is updated
    EXPECT_GT(s->tLastTile, t0);
}

TEST_F(RenderJobSchedulerTests, SetTileCompleteFinishesJob) {
    // completing the final tile in a job finishes it
    //  (nb: no Finalizer connected, so we can verify the state in the local job register)
    cam.setHSize(64);
    cam.setVSize(64);
    const auto id = sched->submit({
        cam, world, JobType::offline
    });
    auto s = sched->getJobState(id);
    ASSERT_NE(s, nullptr);
    EXPECT_FALSE(s->isCompleted.load());
    auto t = sched->getNextTile();
    auto nTiles = s->nTiles;
    auto t0 = std::chrono::steady_clock::now();
    sched->setTileComplete(t.value());
    t = sched->getNextTile();
    sched->setTileComplete(t.value());
    t = sched->getNextTile();
    sched->setTileComplete(t.value());
    EXPECT_EQ(s->nTilesRemain.load(), 1);
    // the final tile completes the job
    t = sched->getNextTile();
    sched->setTileComplete(t.value());
    // completed job state in register since finalizer not yet connected
    s = sched->getJobState(id);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->nTilesRemain.load(), 0);
    EXPECT_EQ(s->nTilesComplete.load(), nTiles);
    EXPECT_TRUE(s->isCompleted.load());
    EXPECT_GT(s->tComplete, t0);
}

TEST_F(RenderJobSchedulerTests, JobIsCancelled) {
    // an in progress job is cancelled and its tiles
    //  are no longer acquired from the queue
    cam.setHSize(256);
    cam.setVSize(256);
    const auto id = sched->submit({
        cam, world, JobType::realtime
    });
    // grab and complete few tiles
    for (int i{ }; i < 3; ++i) {
        auto t = sched->getNextTile();
        auto th = std::jthread{[&]() {
            sched->setTileComplete(t.value());
        }};
    }
    // cancel job
    sched->cancel(id);
    // the job state reflects being cancelled
    auto state = sched->getJobState(id);
    ASSERT_NE(state, nullptr);
    EXPECT_EQ(state->nTilesRemain.load(), 61);
    EXPECT_TRUE(state->isCancelled.load(std::memory_order_relaxed));
    // no more tiles received
    auto t = sched->getNextTile();
    EXPECT_FALSE(t);
    // no tiles remain and the job is marked completed
    auto s = sched->getJobState(id);
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->nTilesRemain.load(), 0);
    EXPECT_TRUE(s->isCompleted.load());
    // job was not fully rendered
    EXPECT_NE(s->nTilesComplete.load(), s->nTiles);
}

/*
 *  RenderImageTarget
 */
class ImageTargetTests: public RenderJobSchedulerTests {
protected:
    void SetUp() override {
        RenderJobSchedulerTests::SetUp();
    }
};

TEST_F(ImageTargetTests, BasicProperties) {
    auto target = ImageTarget{ cam.getHSize(), cam.getVSize() };
    EXPECT_EQ(target.buffer.getWidth(), cam.getHSize());
    EXPECT_EQ(target.buffer.getHeight(), cam.getVSize());
}

/*
 *  RenderSchedulerCompleteAndSummary
 *   - tests Job Summary data structure
 *   - also job completion and summarization on the Scheduler
 */
class RenderSchedulerCompletion: public RenderJobSchedulerTests {
protected:
    JobFinalizer finalizer{ };
    std::shared_ptr<JobState> st{ nullptr };
    Job job{ cam, world, JobType::offline };
    JobID id{ JobID_INVALID };

    void SetUp() override {
        RenderJobSchedulerTests::SetUp();
        id = sched->submit(job);
        st = sched->getJobState(id);
    }
};

TEST_F(RenderSchedulerCompletion, DefaultProperties) {
    auto s = JobSummary{ st->job.target };
    EXPECT_EQ(s.id, JobID_INVALID);
    EXPECT_EQ(s.type, JobType::invalid);
    EXPECT_EQ(s.endReason, JobEndReason::invalid);
}

TEST_F(RenderSchedulerCompletion, MakeJobSummaryFromState) {
    // the JobScheduler::getJobSummary factory constructs a snapshot
    //  JobSummary of a JobState
    auto s = JobScheduler::makeSummary(st);
    EXPECT_EQ(s.id, st->job.id);
    EXPECT_EQ(s.type, st->job.type);
    EXPECT_EQ(s.endReason, JobEndReason::failed);
    EXPECT_EQ(s.target, st->job.target);
    EXPECT_EQ(s.nTiles, st->nTiles);
    EXPECT_EQ(s.nTilesComplete, st->nTilesComplete);
    EXPECT_EQ(s.nPixelsComplete, st->nPixelsComplete);
    EXPECT_EQ(s.nPasses, st->job.passes.size());
    EXPECT_EQ(s.tSubmit, st->tSubmit);
    EXPECT_EQ(s.tStart, st->tStart);
    EXPECT_EQ(s.tComplete, st->tComplete);
    // cancelled state
    st->isCancelled = true;
    s = JobScheduler::makeSummary(st);
    EXPECT_EQ(s.endReason, JobEndReason::cancelled);
    // completed state takes precendence over cancelled
    //  in determining end reason
    st->isCompleted = true;
    s = JobScheduler::makeSummary(st);
    EXPECT_EQ(s.endReason, JobEndReason::completed);
}

/*
 *  RenderJobFinalizer
 */
class RenderJobFinalizerTests: public RenderJobSchedulerTests {
protected:
    JobFinalizer finalizer{ };
    std::shared_ptr<JobState> st{ nullptr };
    Job job{ cam, world, JobType::offline };
    JobID id{ JobID_INVALID };

    void SetUp() override {
        RenderJobSchedulerTests::SetUp();
        id = sched->submit(job);
        st = sched->getJobState(id);
    }
};

TEST_F(RenderJobFinalizerTests, StartsAndStops) {
    // instance is created and the worker thread starts/stops
    EXPECT_FALSE(finalizer.isRunning.load());
    finalizer.start();
    std::this_thread::sleep_for(15ms);
    EXPECT_TRUE(finalizer.isRunning.load());
    finalizer.stop();
    std::this_thread::sleep_for(15ms);
    EXPECT_FALSE(finalizer.isRunning.load());
}

TEST_F(RenderJobFinalizerTests, BasicEnqueueJob) {
    // pushing onto the queue works and the ended job appears in the queue to consume
    auto summary = JobScheduler::makeSummary(st);
    EXPECT_TRUE(finalizer.queue.empty());
    finalizer.push({summary, nullptr});
    EXPECT_FALSE(finalizer.queue.empty());
    const auto ps_rec = finalizer.queue.front();
    ASSERT_NE(ps_rec, nullptr);
    EXPECT_EQ(ps_rec->summary.id, summary.id);
    EXPECT_EQ(ps_rec->summary.target.path, summary.target.path);
}

TEST_F(RenderJobFinalizerTests, EnqueueExecutesCallbacks) {
    // an enqueued job to finalize has its appropriate callbacks executed
    //  by the queue which is running
    auto summary = JobScheduler::makeSummary(st);
    EXPECT_TRUE(finalizer.queue.empty());
    finalizer.start();
    bool cbIsExecuted{ false };
    const auto cb = JobEndedCallback{[&](const auto&){ cbIsExecuted = true; }};
    finalizer.push({summary, cb});
    std::this_thread::sleep_for(10ms);
    finalizer.stop();
    EXPECT_TRUE(cbIsExecuted);
}


/*
 * RenderJobFinalizer - integration with other modules
 */
class RenderJobFinalizerIntegration: public RenderJobFinalizerTests {
protected:

    void SetUp() override {
        RenderJobFinalizerTests::SetUp();
    }
};

TEST_F(RenderJobFinalizerIntegration, AttachFinalizerToScheduler) {
    // the scheduler allows attaching a Finalizer instance
    EXPECT_EQ(sched->finalizer, nullptr);
    EXPECT_EQ(finalizer.scheduler, nullptr);
    sched->attachToFinalizer(finalizer);
    EXPECT_EQ(sched->finalizer, &finalizer);
    // the connection is bidirectional
    EXPECT_NE(finalizer.scheduler, nullptr);
}

TEST_F(RenderJobFinalizerIntegration, FinalizedToDisk) {
    // a job is finalized and rendered to disk as a file
    EXPECT_TRUE(false);
}

TEST_F(RenderJobFinalizerIntegration, FinalizedToBuffer) {
    // a job is finalized to an image buffer (only)
    EXPECT_TRUE(false);
}

TEST_F(RenderJobFinalizerIntegration, GetSummaryOfFinalizedJob) {
    // the Finalizer maintains a registry of finalized jobs
    //  and .getJobSummary() returns the appropriate Summary
    CORE_TRACE("\nTEST START");
    EXPECT_EQ(nullptr, finalizer.getSummary(id));
    auto summary = JobScheduler::makeSummary(st);
    finalizer.start();
    finalizer.push({summary, nullptr});
    std::this_thread::sleep_for(10ms);
    auto res = finalizer.getSummary(id);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->id, summary.id);
    EXPECT_TRUE(finalizer.queue.empty());
}

TEST_F(RenderJobFinalizerIntegration, FinalizesFromScheduler) {
    // the scheduler is connected, and a job is completed on it.
    //   the job is automatically enqueued to the finalizer, which
    //   is appropriately finalized
    sched->attachToFinalizer(finalizer);
    finalizer.start();
    // complete the job by marking all tiles complete
    const auto nTiles = st->nTiles;
    for (uint32_t n{}; n < nTiles; ++n) {
        if (const auto t = sched->getNextTile()) {
            sched->setTileComplete(t.value());
        }
    }
    EXPECT_EQ(st->nTilesComplete, nTiles);
    // summary is now in the finalizer's registry
    std::this_thread::sleep_for(10ms);
    auto summary = finalizer.getSummary(id);
    EXPECT_NE(summary, nullptr);
    // the job is removed from the scheduler's register after finalizing
    auto st_now = sched->getJobState(id);
    EXPECT_EQ(st_now, nullptr);
}


/*
 *  RenderWorker
 */
class RenderWorkerTests: public RenderJobSchedulerTests {
protected:
    std::unique_ptr<Worker> worker{ nullptr };
    static constexpr uint32_t ID{ 2 };
    Job job{ cam, world, JobType::offline };

    void SetUp() override {
        RenderJobSchedulerTests::SetUp();
        worker = std::make_unique<Worker>(ID, *sched);
    }
};

TEST_F(RenderWorkerTests, ConstructedWithDefaults) {
    ASSERT_NE(worker, nullptr);
    EXPECT_EQ(worker->id, ID);
    EXPECT_EQ(worker->thread, nullptr);
    EXPECT_EQ(worker->isRunning.load(), false);
}

TEST_F(RenderWorkerTests, ThreadStartsAndStops) {
    ASSERT_NE(worker, nullptr);
    EXPECT_EQ(worker->thread, nullptr);
    EXPECT_EQ(worker->isRunning.load(), false);
    auto th = std::jthread{[&]() {
        std::this_thread::sleep_for(100ms);
        sched->shutdown();
    }};
    worker->start();
    std::this_thread::sleep_for(20ms);
    EXPECT_NE(worker->thread, nullptr);
    EXPECT_EQ(worker->isRunning.load(), true);
    EXPECT_TRUE(worker->thread->joinable());
    worker->stop();
    std::this_thread::sleep_for(20ms);
    EXPECT_EQ(worker->isRunning.load(), false);
    EXPECT_FALSE(worker->thread->joinable());
}

TEST_F(RenderWorkerTests, ConsumesTileFromScheduler) {
    // Tiles should automatically be loaded into the worker
    //  from the scheduler during .run(), and marked
    //  completed after
    ASSERT_NE(worker, nullptr);
    auto id = sched->submit(job);
    auto s = sched->getJobState(id);
    auto n_init = s->nTilesRemain.load();
    EXPECT_EQ(n_init, 64);
    auto th = std::jthread{[&]() {
        std::this_thread::sleep_for(50ms);
        sched->shutdown();
    }};
    worker->start();
    std::this_thread::sleep_for(10ms);
    auto n_final = s->nTilesRemain.load();
    EXPECT_LT(n_final, n_init);
}

TEST_F(RenderWorkerTests, RendersToFrameBuffer) {
    // a single worker renders tile data to a framebuffer (Canvas)
    ASSERT_NE(worker, nullptr);
    // make it a tiny job
    cam.setHSize(64); cam.setVSize(64);
    auto job = Job{ cam, world, JobType::offline };
    auto id = sched->submit(job);
    auto s = sched->getJobState(id);
    auto n_init = s->nTilesRemain.load();
    EXPECT_EQ(n_init, 4);
    // we're not validating render data here since that's done elsewhere;
    //  just verifying that pixels have been written and state has changed
    EXPECT_TRUE(s->job.target.buffer.isBlank());
    // start rendering
    EXPECT_TRUE(false);
}

