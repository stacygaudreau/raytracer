#include "gtest/gtest.h"
#include "raytracer/renderer/renderer.hpp"
#include "raytracer/environment/camera.hpp"
#include "raytracer/renderer/canvas.hpp"
#include "raytracer/shapes/sphere.hpp"

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



