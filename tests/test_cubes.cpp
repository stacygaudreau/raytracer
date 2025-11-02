#include "gtest/gtest.h"
#include "raytracer/shapes/cube.hpp"
#include "raytracer/renderer/ray.hpp"
#include "raytracer/renderer/intersection.hpp"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Ray/Cube Intersections
////////////////////////////////////////////////////////////////////////////////////////////////////
class RayCubeIntersections: public ::testing::Test
{
  protected:
    Cube c{};
};

TEST_F(RayCubeIntersections, IntersectionOnFace_PosX)
{
    Ray r{ Point{5, .5, 0}, Vector{-1, 0, 0} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4.);
    EXPECT_DOUBLE_EQ(xs(1).t, 6.);
}

TEST_F(RayCubeIntersections, IntersectionOnFace_NegX)
{
    Ray r{ Point{-5, .5, 0}, Vector{1, 0, 0} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4.);
    EXPECT_DOUBLE_EQ(xs(1).t, 6.);
}

TEST_F(RayCubeIntersections, IntersectionOnFace_PosY)
{
    Ray r{ Point{0.5, 5, 0}, Vector{0, -1, 0} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4.);
    EXPECT_DOUBLE_EQ(xs(1).t, 6.);
}

TEST_F(RayCubeIntersections, IntersectionOnFace_NegY)
{
    Ray r{ Point{0.5, -5, 0}, Vector{0, 1, 0} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4.);
    EXPECT_DOUBLE_EQ(xs(1).t, 6.);
}

TEST_F(RayCubeIntersections, IntersectionOnFace_PosZ)
{
    Ray r{ Point{0.5, 0, 5}, Vector{0, 0, -1} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4.);
    EXPECT_DOUBLE_EQ(xs(1).t, 6.);
}

TEST_F(RayCubeIntersections, IntersectionOnFace_NegZ)
{
    Ray r{ Point{0.5, 0, -5}, Vector{0, 0, 1} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4.);
    EXPECT_DOUBLE_EQ(xs(1).t, 6.);
}

TEST_F(RayCubeIntersections, IntersectionInside)
{
    Ray r{ Point{0, 0.5, 0}, Vector{0, 0, 1} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, -1.);
    EXPECT_DOUBLE_EQ(xs(1).t, 1.);
}

TEST_F(RayCubeIntersections, RayMissesCube_0)
{
    Ray r{ Point{-2, 0, 0}, Vector{0.2673, 0.5345, 0.8018} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(RayCubeIntersections, RayMissesCube_1)
{
    Ray r{ Point{0, -2, 0}, Vector{0.8018, 0.2673, 0.5345} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(RayCubeIntersections, RayMissesCube_2)
{
    Ray r{ Point{0, 0, -2}, Vector{0.5345, 0.8018, 0.2673} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(RayCubeIntersections, RayMissesCube_3)
{
    Ray r{ Point{2, 0, 2}, Vector{0, 0, -1} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(RayCubeIntersections, RayMissesCube_4)
{
    Ray r{ Point{0, 2, 2}, Vector{0, -1, 0} };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(RayCubeIntersections, RayMissesCube_5)
{
    Ray r{ Point{ 2, 2, 0 }, Vector{ -1, 0, 0 } };
    Intersections xs = c.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(RayCubeIntersections, NormalAtPoint_0)
{
    // the normal vector is computed at various points on the cube
    Point p{ 1, 0.5, -0.8 };
    Vector normal{ 1, 0, 0 };
    EXPECT_EQ(c.normalAt(p), normal);
}

TEST_F(RayCubeIntersections, NormalAtPoint_1)
{
    Point p{ -1, -0.2, 0.9 };
    Vector normal{ -1, 0, 0 };
    EXPECT_EQ(c.normalAt(p), normal);
}

TEST_F(RayCubeIntersections, NormalAtPoint_2)
{
    Point p{ -0.4, 1, -0.1 };
    Vector normal{ 0, 1, 0 };
    EXPECT_EQ(c.normalAt(p), normal);
}

TEST_F(RayCubeIntersections, NormalAtPoint_3)
{
    Point p{ 0.3, -1, -0.7 };
    Vector normal{ 0, -1, 0 };
    EXPECT_EQ(c.normalAt(p), normal);
}

TEST_F(RayCubeIntersections, NormalAtPoint_4)
{
    Point p{ -0.6, 0.3, 1 };
    Vector normal{ 0, 0, 1 };
    EXPECT_EQ(c.normalAt(p), normal);
}

TEST_F(RayCubeIntersections, NormalAtPoint_5)
{
    Point p{ 0.4, 0.4, -1 };
    Vector normal{ 0, 0, -1 };
    EXPECT_EQ(c.normalAt(p), normal);
}

TEST_F(RayCubeIntersections, NormalAtPoint_6)
{
    Point p{ 1, 1, 1 };
    Vector normal{ 1, 0, 0 };
    EXPECT_EQ(c.normalAt(p), normal);
}
TEST_F(RayCubeIntersections, NormalAtPoint_7)
{
    Point p{ -1, -1, -1 };
    Vector normal{ -1, 0, 0 };
    EXPECT_EQ(c.normalAt(p), normal);
}
