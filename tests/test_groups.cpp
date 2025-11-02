#include "raytracer/shapes/group.hpp"
#include "raytracer/shapes/sphere.hpp"
#include "gtest/gtest.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Group Basics
////////////////////////////////////////////////////////////////////////////////////////////////////
class GroupBasics: public ::testing::Test
{
  protected:
    Group g{};

    // test shape
    class TestShape: public Shape
    {
      public:
        TestShape(): Shape(){};

        Tuple localNormalAt(Tuple localPoint, Intersection iHit = {}) override
        {
            (void)iHit;
            return Vector{ localPoint.x, localPoint.y, localPoint.z };
        };

        Intersections localIntersect(Ray localRay) override
        {
            objectRay = localRay;
            return Intersections{};
        }

        Ray objectRay{Tuple{}, Tuple{}};
    };
};

TEST_F(GroupBasics, CreatingAGroup)
{
    EXPECT_EQ(g.getTransform(), TransformationMatrix::identity());
    EXPECT_TRUE(g.isEmpty());
}

TEST_F(GroupBasics, ShapeHasParentAttribute)
{
    TestShape s{};
    EXPECT_FALSE(g.isGrouped());
    EXPECT_FALSE(s.isGrouped());
}

TEST_F(GroupBasics, AddingChildToGroup)
{
    // makes the group the child's parent, and the child is added to the group collection
    TestShape s{};
    g.addChild(&s);
    EXPECT_FALSE(g.isEmpty());
    EXPECT_EQ(g.getChild(0), s);
    EXPECT_TRUE(s.isGrouped());
    EXPECT_EQ(s.getGroup(), g);
}

TEST_F(GroupBasics, IntersectEmptyGroup)
{
    Ray r{ Point{0, 0, 0}, Vector{0, 0, 1} };
    Intersections xs = g.localIntersect(r);
    EXPECT_TRUE(xs.isEmpty());
}

TEST_F(GroupBasics, IntersectNonEmptyGroup)
{
    Sphere s1{};
    Sphere s2{};
    s2.setTransform(Transform::translation(0., 0., -3.));
    Sphere s3{};
    s3.setTransform(Transform::translation(5., 0., 0.));
    g.addChild(&s1);
    g.addChild(&s2);
    g.addChild(&s3);
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    Intersections xs = g.localIntersect(r);
    EXPECT_EQ(xs.count(), 4);
    EXPECT_EQ(xs(0).shape, &s2);
    EXPECT_EQ(xs(1).shape, &s2);
    EXPECT_EQ(xs(2).shape, &s1);
    EXPECT_EQ(xs(3).shape, &s1);
}

TEST_F(GroupBasics, IntersectingTransformedGroup)
{
    g.setTransform(Transform::scale(2., 2., 2.));
    Sphere s{};
    s.setTransform(Transform::translation(5., 0., 0.));
    g.addChild(&s);
    Ray r{ Point{10, 0, -10}, Vector{0, 0, 1} };
    Intersections xs = g.intersect(r);
    EXPECT_EQ(xs.count(), 2);
}