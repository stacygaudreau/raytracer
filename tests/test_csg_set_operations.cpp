#include "gtest/gtest.h"
#include "raytracer/rays.h"
#include "raytracer/intersection.h"
#include "raytracer/sphere.h"
#include "raytracer/cube.h"
#include "raytracer/utils.h"
#include "raytracer/csg.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSG Operations
////////////////////////////////////////////////////////////////////////////////////////////////////
class ConstructiveSolidGeometry: public ::testing::Test
{
  protected:

    Sphere s1{};
    Cube s2{};
    Intersections xs{{
        {1.0, &s1},
        {2.0, &s2},
        {3.0, &s1},
        {4.0, &s2}
    }};

    void SetUp() override
    {
    }
};

TEST_F(ConstructiveSolidGeometry, ConstructCSGShape)
{
    // an operation and two shapes create a CSG
    auto c = CSG::Union(&s1, &s2);
    EXPECT_EQ(c.getOperation(), CSG::Operation::UNION);
    EXPECT_EQ(c.getLeft(), s1);
    EXPECT_EQ(c.getRight(), s2);
    EXPECT_EQ(s1.getGroup(), c);
    EXPECT_EQ(s2.getGroup(), c);
}

TEST_F(ConstructiveSolidGeometry, UnionPreservesShapeIntersections)
{
    // the exterior intersections of each shape are preserved by the union
    //  (and the rest of the intersections, ie: not on the exterior, are ignored)
    CSG c{ &s1, &s2 };
    EXPECT_TRUE(c.intersectionAllowed(true, true, false));
    EXPECT_TRUE(c.intersectionAllowed(false, false, true));
    EXPECT_FALSE(c.intersectionAllowed(false, true, true));
    EXPECT_FALSE(c.intersectionAllowed(true, true, true));
    EXPECT_TRUE(c.intersectionAllowed(false, false, false));
}

TEST_F(ConstructiveSolidGeometry, IntersectPreservesOverlappingIntersections)
{
    // ray intersections are chosen such that they preserve only the common volume
    //  btwn. the two shapes
    auto csg = CSG::Intersect(&s1, &s2);
    EXPECT_TRUE(csg.intersectionAllowed(true, true, true));
    EXPECT_FALSE(csg.intersectionAllowed(true, true, false));
    EXPECT_TRUE(csg.intersectionAllowed(true, false, true));
    EXPECT_FALSE(csg.intersectionAllowed(true, false, false));
    EXPECT_TRUE(csg.intersectionAllowed(false, true, true));
    EXPECT_TRUE(csg.intersectionAllowed(false, true, false));
    EXPECT_FALSE(csg.intersectionAllowed(false, false, true));
    EXPECT_FALSE(csg.intersectionAllowed(false, false, false));
}

TEST_F(ConstructiveSolidGeometry, DifferencePreservesIntsNotOnlyInRightShape)
{
    auto csg = CSG::Difference(&s1, &s2);
    EXPECT_FALSE(csg.intersectionAllowed(true, true, true));
    EXPECT_TRUE(csg.intersectionAllowed(true, true, false));
    EXPECT_FALSE(csg.intersectionAllowed(true, false, true));
    EXPECT_TRUE(csg.intersectionAllowed(true, false, false));
    EXPECT_TRUE(csg.intersectionAllowed(false, true, true));
    EXPECT_TRUE(csg.intersectionAllowed(false, true, false));
    EXPECT_FALSE(csg.intersectionAllowed(false, false, true));
    EXPECT_FALSE(csg.intersectionAllowed(false, false, false));
}

TEST_F(ConstructiveSolidGeometry, FilteringListOfIntersections_Union)
{
    // a subset of intersections is filtered in order to conform to a particular
    // CSG set operation
    auto csg = CSG::Union(&s1, &s2);
    auto res = csg.filterIntersections(xs);
    EXPECT_EQ(res.count(), 2);
    EXPECT_EQ(res(0), xs(0));
    EXPECT_EQ(res(1), xs(3));
}

TEST_F(ConstructiveSolidGeometry, FilteringListOfIntersections_Intersect)
{
    auto csg = CSG::Intersect(&s1, &s2);
    auto res = csg.filterIntersections(xs);
    EXPECT_EQ(res.count(), 2);
    EXPECT_EQ(res(0), xs(1));
    EXPECT_EQ(res(1), xs(2));
}

TEST_F(ConstructiveSolidGeometry, FilteringListOfIntersections_Difference)
{
    auto csg = CSG::Difference(&s1, &s2);
    auto res = csg.filterIntersections(xs);
    EXPECT_EQ(res.count(), 2);
    EXPECT_EQ(res(0), xs(0));
    EXPECT_EQ(res(1), xs(1));
}

TEST_F(ConstructiveSolidGeometry, RayMissesCSGShape)
{
    auto csg = CSG::Union(&s1, &s2);
    Ray r{ Point{0, 2, -5}, Vector{0, 0, 1} };
    auto xs = csg.localIntersect(r);
    EXPECT_TRUE(xs.isEmpty());
}

TEST_F(ConstructiveSolidGeometry, RayHitsCSGShape)
{
    Sphere s1{}, s2{};
    s2.setTransform(Transform::translation(0., 0., 0.5));
    auto csg = CSG::Union(&s1, &s2);
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    auto xs = csg.localIntersect(r);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_EQ(xs(0).t, 4.0);
    EXPECT_EQ(xs(0).shape, &s1);
    EXPECT_EQ(xs(1).t, 6.5);
    EXPECT_EQ(xs(1).shape, &s2);
}
