#include "gtest/gtest.h"
#include "raytracer/common/utils.hpp"
#include "raytracer/shapes/plane.hpp"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// PlaneBasics
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(PlaneBasics, NormalOfPlaneIsConstantEverywhere)
{
    Plane p{};
    auto n1 = p.localNormalAt(Point{0, 0, 0}, {});
    auto n2 = p.localNormalAt(Point{10, 0, -10}, {});
    auto n3 = p.localNormalAt(Point{-5, 0, 150}, {});
    const auto normal = Vector{0, 1, 0};
    EXPECT_EQ(n1, normal);
    EXPECT_EQ(n2, normal);
    EXPECT_EQ(n3, normal);
}

TEST(PlaneBasics, RayParallelToPlane)
{
    // a ray parallel to the plane will never intersect it
    Plane p{};
    Ray r{Point{0, 10, 0}, Vector{0, 0, 1}};
    auto intersections = p.localIntersect(r);
    EXPECT_TRUE(intersections.isEmpty());
}

TEST(PlaneBasics, RayCoplanarToPlane)
{
    // although technically this would result in infinite intersections,
    //  we consider it a miss since it would be too thin to render on screen anyway
    Plane p{};
    Ray r{Point{0, 0, 0}, Vector{0, 0, 1}};
    auto intersections = p.localIntersect(r);
    EXPECT_TRUE(intersections.isEmpty());
}

TEST(PlaneBasics, RayIntersectingPlaneFromAbove)
{
    Plane p{};
    Ray r{Point{0, 1, 0}, Vector{0, -1, 0}};
    auto intersections = p.localIntersect(r);
    EXPECT_EQ(intersections.count(), 1);
    EXPECT_EQ(intersections(0).t, 1);
    EXPECT_EQ(intersections(0).shape, &p);
}

TEST(PlaneBasics, RayIntersectingPlaneFromBelow)
{
    Plane p{};
    Ray r{Point{0, -1, 0}, Vector{0, 1, 0}};
    auto intersections = p.localIntersect(r);
    EXPECT_EQ(intersections.count(), 1);
    EXPECT_EQ(intersections(0).t, 1);
    EXPECT_EQ(intersections(0).shape, &p);
}







