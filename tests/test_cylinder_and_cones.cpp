#include "gtest/gtest.h"
#include "raytracer/shapes/cylinder.hpp"
#include "raytracer/shapes/cone.hpp"
#include "raytracer/renderer/ray.hpp"
#include "raytracer/renderer/intersection.hpp"
#include "raytracer/common/utils.hpp"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Cylinders
////////////////////////////////////////////////////////////////////////////////////////////////////
class Cylinders: public ::testing::Test
{
  protected:
    Cylinder cyl{};

    void SetUp() override
    {
    }
};

TEST_F(Cylinders, RayMissesCylinder_0)
{
    auto dir = Vector{0, 1, 0}.normalize();
    Ray r{ Point{1, 0, 0}, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, RayMissesCylinder_1)
{
    auto dir = Vector{0, 1, 0}.normalize();
    Ray r{ Point{0, 0, 0}, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, RayMissesCylinder_2)
{
    auto dir = Vector{1, 1, 1}.normalize();
    Ray r{ Point{0, 0, -5}, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, RayHitsCylinder_0)
{
    auto dir = Vector{ 0, 0, 1 }.normalize();
    Ray r{ Point{ 1, 0, -5 }, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 5.0);
    EXPECT_DOUBLE_EQ(xs(1).t, 5.0);
}

TEST_F(Cylinders, RayHitsCylinder_1)
{
    auto dir = Vector{ 0, 0, 1 }.normalize();
    Ray r{ Point{ 0, 0, -5 }, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4.0);
    EXPECT_DOUBLE_EQ(xs(1).t, 6.0);
}

TEST_F(Cylinders, RayHitsCylinder_2)
{
    auto dir = Vector{ 0.1, 1, 1 }.normalize();
    Ray r{ Point{ 0.5, 0, -5 }, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_TRUE(APPROX_EQ(xs(0).t, 6.80798));
    EXPECT_TRUE(APPROX_EQ(xs(1).t, 7.08872));
}

TEST_F(Cylinders, NormalVector_PosX)
{
    // chooses 4 pts at each +x, -x, +z and -z, evaluating the normal
    auto p = Point{ 1, 0, 0 };
    auto n = Vector{ 1, 0, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVector_NegX)
{
    auto p = Point{ -1, 1, 0 };
    auto n = Vector{ -1, 0, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVector_PosZ)
{
    auto p = Point{ 0, -2, 1 };
    auto n = Vector{ 0, 0, 1 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVector_NegZ)
{
    auto p = Point{ 0, 5, -1 };
    auto n = Vector{ 0, 0, -1 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, DefaultMinMaxBounds)
{
    // the default minimum and maxY bounds for an infinite cylinder

    EXPECT_DOUBLE_EQ(cyl.getMinY(), -INF);
    EXPECT_DOUBLE_EQ(cyl.getMaxY(), INF);
}

TEST_F(Cylinders, IntersectingConstrainedCyl_0)
{
    // cast diagonally from inside cyl, escaping without intersecting
    cyl.setHeight(2.0, 1.0);
    auto p = Point{ 0, 1.5, 0 };
    auto dir = Vector{ 0.1, 1.0, 0 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, IntersectingConstrainedCyl_1)
{
    // perpendicular to y axis, above and below cyl.
    cyl.setHeight(2.0, 1.0);
    auto p = Point{ 0, 3, -5 };
    auto dir = Vector{ 0, 0, 1 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, IntersectingConstrainedCyl_2)
{
    // perpendicular to y axis, above and below cyl.
    cyl.setHeight(2.0, 1.0);
    auto p = Point{ 0, 0, -5 };
    auto dir = Vector{ 0, 0, 1 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, IntersectingConstrainedCyl_3)
{
    // edge case, shows min/max are themselves outside the bounds of cylinder
    cyl.setHeight(2.0, 1.0);
    auto p = Point{ 0, 2, -5 };
    auto dir = Vector{ 0, 0, 1 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, IntersectingConstrainedCyl_4)
{
    // edge case, shows min/max are themselves outside the bounds of cylinder
    cyl.setHeight(2.0, 1.0);
    auto p = Point{ 0, 1, -5 };
    auto dir = Vector{ 0, 0, 1 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 0);
}

TEST_F(Cylinders, IntersectingConstrainedCyl_5)
{
    // perpendicular thru middle of cylinder, producing two intersections
    cyl.setHeight(2.0, 1.0);
    auto p = Point{ 0, 1.5, -2 };
    auto dir = Vector{ 0, 0, 1 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
}

TEST_F(Cylinders, ClosedAttribute)
{
    // cylinders are by default tubes, without closed end caps.
    EXPECT_FALSE(cyl.getIsClosed());
}

TEST_F(Cylinders, IntersectingCapsOfClosedCyl_0)
{
    // cast above cyl, pointing down through middle along y axis
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 3, 0 };
    auto dir = Vector{ 0, -1, 0 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
}

TEST_F(Cylinders, IntersectingCapsOfClosedCyl_1)
{
    // cast diagonally from above/below
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 3, -2 };
    auto dir = Vector{ 0, -1, 2 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
}

TEST_F(Cylinders, IntersectingCapsOfClosedCyl_2)
{
    // cast diagonally from above/below
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 0, -2 };
    auto dir = Vector{ 0, 1, 2 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
}

TEST_F(Cylinders, IntersectingCapsOfClosedCyl_3)
{
    // corner case that could generate more intersections, but should be == 2
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 4, -2 };
    auto dir = Vector{ 0, -1, 1 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
}

TEST_F(Cylinders, IntersectingCapsOfClosedCyl_4)
{
    // corner case that could generate more intersections, but should be == 2
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, -1, -2 };
    auto dir = Vector{ 0, 1, 1 }.normalize();
    Ray r{ p, dir };
    Intersections xs = cyl.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
}

TEST_F(Cylinders, NormalVectorOnCaps_0)
{
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 1, 0 };
    auto n = Vector{ 0, -1, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVectorOnCaps_1)
{
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ .5, 1, 0 };
    auto n = Vector{ 0, -1, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVectorOnCaps_2)
{
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 1, 0.5 };
    auto n = Vector{ 0, -1, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVectorOnCaps_3)
{
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 2, 0 };
    auto n = Vector{ 0, 1, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVectorOnCaps_4)
{
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0.5, 2, 0 };
    auto n = Vector{ 0, 1, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}

TEST_F(Cylinders, NormalVectorOnCaps_5)
{
    cyl.setHeight(2.0, 1.0);
    cyl.setIsClosed(true);
    auto p = Point{ 0, 2, 0.5 };
    auto n = Vector{ 0, 1, 0 };
    EXPECT_EQ(cyl.localNormalAt(p), n);
}
