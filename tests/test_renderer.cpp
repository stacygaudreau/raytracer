#include "gtest/gtest.h"
#include "raytracer/renderer/renderer.hpp"
#include "raytracer/environment/camera.hpp"
#include "raytracer/renderer/canvas.hpp"
#include "raytracer/shapes/sphere.hpp"
#include "raytracer/logging/logging.hpp"

using namespace rt;
using namespace rt::Render;


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
        job = std::make_unique<Job>(camera, world);
    }
};

TEST_F(RenderJobTests, ConstructedWithProperties) {
    EXPECT_EQ(job->id, JobID_INVALID);
    EXPECT_EQ(job->type, JobType::invalid);
    EXPECT_EQ(job->passes.size(), 1);
    EXPECT_EQ(job->passes.at(0), 1);
    EXPECT_EQ(job->width, camera.getHSize());
    EXPECT_EQ(job->height, camera.getVSize());
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
    EXPECT_EQ(state->nTilesRemain, 0);
    EXPECT_EQ(state->status, Status::invalid);
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

TEST_F(RenderJobSchedulerTests, ConstructedWithProperties) {
    EXPECT_NE(sched, nullptr);
}

TEST_F(RenderJobSchedulerTests, GetEvenTilesForSinglePassSquareViewportJob) {
    // a single pass job is broken up into a set of tiles
    //  from a camera with a perfectly square viewport with EVEN tile size
    cam.setHSize(256); cam.setVSize(256);
    Job job{ cam, world };
    job.id = 12345;
    job.type = JobType::background;
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
    Job job{ cam, world };
    job.type = JobType::background;
    auto state = std::make_shared<JobState>(job);
    auto tiles = sched->getTilesForJobState(state, 32);
    EXPECT_EQ(tiles.size(), 1);
    // tileSize is exactly the image size
    cam.setHSize(32); cam.setVSize(32);
    Job job2{ cam, world };
    job2.type = JobType::background;
    auto state2 = std::make_shared<JobState>(job2);
    tiles = sched->getTilesForJobState(state2, 32);
    EXPECT_EQ(tiles.size(), 1);
    // tileSize is larger than the actual image on one axis
    cam.setHSize(33); cam.setVSize(32);
    Job job3{ cam, world };
    job3.type = JobType::background;
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
    Job job{ cam, world };
    job.type = JobType::background;
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
    Job job{ cam, world };
    job.id = 12345;
    job.passes = { 16, 8, 4, 1 };
    job.type = JobType::background;
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
    Job job{ cam, world };
    job.id = 12345;
    job.type = JobType::offline;
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


/*
 *  RenderWorker
 */
class RenderWorkerTests: public RenderEngineTests {
protected:
    JobScheduler scheduler{ };
    std::unique_ptr<Worker> worker{ nullptr };

    void SetUp() override {
        RenderEngineTests::SetUp();
    }
};

TEST_F(RenderWorkerTests, ConstructedWithDefaults) {
    auto worker = std::make_unique<Worker>();
    EXPECT_NE(worker, nullptr);
}

