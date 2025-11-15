#include "gtest/gtest.h"
#include "raytracer/environment/camera.hpp"
#include "raytracer/math/matrix.hpp"
#include "raytracer/common/utils.hpp"
#include "raytracer/shapes/sphere.hpp"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// CameraBasics
////////////////////////////////////////////////////////////////////////////////////////////////////
class CameraBasics: public ::testing::Test
{
  protected:
    World defaultWorld{};
    PointLight light{Point{-10, 10, -10}, Colour{1, 1, 1}};
    Material m1{{0.8, 1.0, 0.6}, 0.1, 0.7, 0.2};
    Sphere s1{}, s2{};

    void SetUp() override {
        s1.setMaterial(m1);
        s2.setTransform(Transform::scale(.5, .5, .5));
        defaultWorld.addLight(PointLight{Point{-10, 10, -10}, Colour{1, 1, 1}});
        defaultWorld.addShape(&s1);
        defaultWorld.addShape(&s2);
    }
};

TEST_F(CameraBasics, CameraIsConstructed)
{
    auto c = Camera{ 160, 120, HALF_PI };
    EXPECT_EQ(c.getHSize(), 160);
    EXPECT_EQ(c.getVSize(), 120);
    EXPECT_DOUBLE_EQ(c.getFOV(), HALF_PI);
    EXPECT_EQ(c.getTransform(), TransformationMatrix::identity());
}

TEST_F(CameraBasics, PixelSizeForHorizontalCanvas)
{
    auto c = Camera{ 200, 125, HALF_PI };
    EXPECT_TRUE(c.getAspectIsHorizontal());
    EXPECT_DOUBLE_EQ(c.getPixelSize(), 0.01);
}

TEST_F(CameraBasics, PixelSizeForVerticalCanvas)
{
    auto c = Camera{ 125, 200, HALF_PI };
    EXPECT_FALSE(c.getAspectIsHorizontal());
    EXPECT_DOUBLE_EQ(c.getPixelSize(), 0.01);
}

TEST_F(CameraBasics, RayThroughCanvasCentre)
{
    // generate a ray from the camera which casts through its canvas centre
    auto c = Camera{ 201, 101, HALF_PI };
    auto r = c.getRayForCanvasPixel(100., 50.);
    EXPECT_EQ(r.getOrigin(), (Point{0., 0., 0.}));
    EXPECT_EQ(r.getDirection(), (Vector{0., 0., -1.}));
}

TEST_F(CameraBasics, RayThroughCanvasTopLeft)
{
    // make a ray from camera to top-left corner of its canvas
    auto c = Camera{ 201, 101, HALF_PI };
    auto r = c.getRayForCanvasPixel(0., 0.);
    EXPECT_EQ(r.getOrigin(), (Point{}));
    auto dir = Vector{ 0.66519, 0.33259, -0.66851 };
    EXPECT_EQ(r.getDirection(), dir);
}

TEST_F(CameraBasics, RayThroughTransformedCameraCanvas)
{
    // here we test a camera canvas ray after transforming the camera around
    auto c = Camera{ 201, 101, HALF_PI };
    auto T = Transform::rotateY(QUARTER_PI) * Transform::translation(0., -2., 5.);
    c.setTransform(T);
    EXPECT_EQ(c.getTransform(), T);
    auto r = c.getRayForCanvasPixel(100., 50.);
    EXPECT_EQ(r.getOrigin(), (Point{0., 2., -5.})); // it's actually the world that moves; not the camera (sign flippage)
    auto dir = Vector{ HALF_SQRT_2, 0., -HALF_SQRT_2 };
    EXPECT_EQ(r.getDirection(), dir);
}

//TEST_F(CameraBasics, CameraRendersWorldCanvas)
//{
//    // renders the default World with a camera, making sure
//    // the pixel is in the right place and colour on the resultant canvas
//    auto c = Camera{ 11, 11, HALF_PI };
//    auto from = Point{ 0., 0., -5. };
//    auto to = Point{};
//    auto up = Vector{0., 1., 0.};
//    c.setTransform(Transform::viewTransform(from, to, up));
//    Canvas image = c.render(defaultWorld);
//    const Colour pix{ 0.38066, 0.47583, 0.2855 };
//    EXPECT_EQ(image.pixelAt(5, 5), pix);
//}

