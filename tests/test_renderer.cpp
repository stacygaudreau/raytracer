#include "gtest/gtest.h"
#include "renderer.h"
#include "camera.h"
#include "canvas.h"
#include "sphere.h"

using namespace rt;

/////////////////////////////////////////////////////////////////////////////////////////////////
// Rendering
/////////////////////////////////////////////////////////////////////////////////////////////////
class Rendering: public ::testing::Test
{
  protected:
    World defaultWorld{};
    PointLight light{Point{-10, 10, -10}, Colour{1, 1, 1}};
    Material m1{{0.8, 1.0, 0.6}, 0.1, 0.7, 0.2};
    Sphere s1{}, s2{};

    Camera camera{ 11, 11, HALF_PI };

    void SetUp() override {
        s1.setMaterial(m1);
        s2.setTransform(Transform::scale(.5, .5, .5));
        defaultWorld.addLight(PointLight{Point{-10, 10, -10}, Colour{1, 1, 1}});
        defaultWorld.addShape(&s1);
        defaultWorld.addShape(&s2);
        // camera orientation
        auto from = Point{ 0., 0., -5. };
        auto to = Point{};
        auto up = Vector{ 0., 1., 0. };
        camera.setTransform(Transform::viewTransform(from, to, up));
    }
};

TEST_F(Rendering, DeterminesOddJobSize)
{
    // the job size in pixels sums up to exactly the number of pixels
    // in the image, for both even and odd divisions of thread count
    Renderer renderer{ 10 };
    renderer.setImageSize(11, 11);
    const size_t px = 121;
    std::vector<size_t> jobsize;
    for (size_t i{}; i < 10; ++i)
        jobsize.push_back(renderer.getJobSize(i));
    EXPECT_EQ(jobsize[0], 13);
    EXPECT_EQ(jobsize[1], 12);
    EXPECT_EQ(jobsize[9], 12);
    size_t total{};
    for (auto size: jobsize)
        total += size;
    EXPECT_EQ(total, px);
}

TEST_F(Rendering, DeterminesEvenJobSize)
{
    Renderer renderer{ 4 };
    renderer.setImageSize(1080, 1080);
    const size_t size = 1166400;
    auto n0 = renderer.getJobSize(0);
    auto n1 = renderer.getJobSize(1);
    auto n2 = renderer.getJobSize(2);
    auto n3 = renderer.getJobSize(3);
    EXPECT_EQ(n0, 291600);
    EXPECT_EQ(n1, 291600);
    EXPECT_EQ(n2, 291600);
    EXPECT_EQ(n3, 291600);
    EXPECT_EQ(n0+n1+n2+n3, size);
}

TEST_F(Rendering, GetPixelXYFromIndex)
{
    // the x and y coords in the pixel matrix are determined from the total index
    // of the pixel in the matrix, and the matrix's dimensions.
    // our test case has an 11x6 matrix
    Camera cam2{ 11, 6, HALF_PI };
    auto image = Canvas{ cam2.getHSize(), cam2.getVSize() };
    auto renderThread = RenderThread{ 0, cam2.getVSize()*cam2.getHSize(), 1,
                                      cam2, defaultWorld, image };
    auto x = renderThread.getPixelX(0);
    auto y = renderThread.getPixelY(0);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 0);

    x = renderThread.getPixelX(3);
    y = renderThread.getPixelY(3);
    EXPECT_EQ(x, 3);
    EXPECT_EQ(y, 0);

    x = renderThread.getPixelX(52);
    y = renderThread.getPixelY(52);
    EXPECT_EQ(x, 8);
    EXPECT_EQ(y, 4);

    x = renderThread.getPixelX(59);
    y = renderThread.getPixelY(59);
    EXPECT_EQ(x, 4);
    EXPECT_EQ(y, 5);

    x = renderThread.getPixelX(65);
    y = renderThread.getPixelY(65);
    EXPECT_EQ(x, 10);
    EXPECT_EQ(y, 5);
}

TEST_F(Rendering, GetOverallImagePixelIndex)
{
    // a pixel index within the worker's job is converted to an index in the
    // overall canvas image's matrix dimensions
    Renderer renderer{ 10 };
    renderer.setImageSize(camera.getHSize(), camera.getVSize());
    auto image = Canvas{ camera.getHSize(), camera.getVSize() };

    // thread 0 handles 13px
    auto n0 = renderer.getJobSize(0);
    EXPECT_EQ(n0, 13);
    auto thread0 = RenderThread{ 0, n0, 10, camera, defaultWorld, image };
    std::vector<size_t> t0_pixels{ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 };
    std::vector<size_t> t0_res{};
    for (size_t i{}; i < n0; ++i)
    {
        t0_res.push_back(thread0.getPixelIndex(i));
        EXPECT_EQ(t0_pixels[i], t0_res[i]);
    }

    // thread 9 handles 12px
    auto n9 = renderer.getJobSize(9);
    EXPECT_EQ(n9, 12);
    auto thread9 = RenderThread{ 9, n9, 10, camera, defaultWorld, image };
    std::vector<size_t> t9_pixels{ 9, 19, 29, 39, 49, 59, 69, 79, 89, 99, 109, 119, 129 };
    std::vector<size_t> t9_res{};
    for (size_t i{}; i < n9; ++i)
    {
        t9_res.push_back(thread9.getPixelIndex(i));
        EXPECT_EQ(t9_pixels[i], t9_res[i]);
    }
}

TEST_F(Rendering, SingleThreadRendersWorldCanvas)
{
    // the default World is rendered by a single worker thread, making sure
    // the resulting pixel has the right location and colour on the
    // resultant image canvas
    auto image = Canvas{ camera.getHSize(), camera.getVSize() };
    auto renderThread = RenderThread{ 0, camera.getVSize()*camera.getHSize(), 1,
                                      camera, defaultWorld, image };
    renderThread.start();
    renderThread.thread.join();

    const Colour pix{ 0.38066, 0.47583, 0.2855 };
    EXPECT_EQ(image.pixelAt(5, 5), pix);
}

TEST_F(Rendering, MultipleThreadsRenderWorldCanvas)
{
    // multiple threads render the default world's canvas (same pixel/loc tests
    //  as single thread test)
    Renderer renderer{ 4 };
    auto renderedImage = renderer.render(camera, defaultWorld);

    const Colour pix{ 0.38066, 0.47583, 0.2855 };
    EXPECT_EQ(renderedImage.pixelAt(5, 5), pix);
}

