#include "gtest/gtest.h"
#include "raytracer/renderer/intersection.hpp"
#include "raytracer/renderer/ray.hpp"
#include "raytracer/shapes/shape.hpp"
#include "raytracer/shapes/sphere.hpp"
#include "raytracer/math/matrix.hpp"
#include "raytracer/shapes/plane.hpp"
#include "raytracer/environment/world.hpp"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Ray Basics
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(Raycasting, RayIsConstructed)
{
    auto origin = Point(1, 2, 3);
    auto dir = Vector(4, 5, 6);
    auto r = Ray(origin, dir);
    EXPECT_EQ(origin, r.getOrigin());
    EXPECT_EQ(dir, r.getDirection());
}

TEST(Raycasting, ComputePointFromDistance)
{
    auto ray = Ray(Point{2, 3, 4}, Vector{1, 0, 0});
    EXPECT_EQ(ray.position(0), Point(2, 3, 4));
    EXPECT_EQ(ray.position(1), Point(3, 3, 4));
    EXPECT_EQ(ray.position(-1), Point(1, 3, 4));
    EXPECT_EQ(ray.position(2.5), Point(4.5, 3, 4));
}

TEST(Raycasting, IntersectsSphereAtTwoPoints)
{
    Ray ray{Point{0, 0, -5}, Vector{0, 0, 1}};
    Sphere s{};
    auto xs = s.intersect(ray);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 4);
    EXPECT_DOUBLE_EQ(xs(1).t, 6);
}

TEST(Raycasting, TangentialIntersectionOfSphere)
{
    Ray ray{Point{0, 1, -5}, Vector{0, 0, 1}};
    Sphere s{};
    auto xs = s.intersect(ray);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, 5);
    EXPECT_DOUBLE_EQ(xs(1).t, 5);
}

TEST(Raycasting, RayMissesASphere)
{
    Ray ray{Point{0, 2, -5}, Vector{0, 0, 1}};
    Sphere s{};
    auto xs = s.intersect(ray);
    EXPECT_EQ(xs.count(), 0);
}

TEST(Raycasting, RayOriginInsideSphere)
{
    Ray ray{Point{0, 0, 0}, Vector{0, 0, 1}};
    Sphere s{};
    auto xs = s.intersect(ray);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, -1);
    EXPECT_DOUBLE_EQ(xs(1).t, 1);
}

TEST(Raycasting, RayIsBehindSphere)
{
    Ray ray{Point{0, 0, 5}, Vector{0, 0, 1}};
    Sphere s{};
    auto xs = s.intersect(ray);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_DOUBLE_EQ(xs(0).t, -6);
    EXPECT_DOUBLE_EQ(xs(1).t, -4);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Intersections
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(Intersections, IntersectionEncapsulatesObjectAndT)
{
    // the intersection datatype should encapsulate
    // the geometric object and also the time of intersection
    Sphere s{};
    Intersection i{3.5, &s};
    EXPECT_EQ(i.t, 3.5);
    EXPECT_EQ(i.shape, &s);
}

TEST(Intersections, IntersectSetsObjectOnIntersection)
{
    Ray ray{Point{0, 0, -5}, Vector{0, 0, 1}};
    Sphere s{};
    auto xs = s.intersect(ray);
    EXPECT_EQ(xs.count(), 2);
    EXPECT_EQ(*xs(0).shape, s);
    EXPECT_EQ(*xs(1).shape, s);
}

TEST(Intersections, SortedByIncreasingT)
{
    Sphere s{};
    Intersection i1{5, &s};
    Intersection i2{7, &s};
    Intersection i3{-3, &s};
    Intersection i4{2, &s};
    Intersections xs{ {i1, i2, i3, i4} };
    EXPECT_EQ(xs(0).t, -3);
    EXPECT_EQ(xs(1).t, 2);
    EXPECT_EQ(xs(2).t, 5);
    EXPECT_EQ(xs(3).t, 7);
}

TEST(Intersections, StaticSortMethod)
{
    Sphere s{};
    std::vector<Intersection> ints{
        {5, &s}, {7, &s}, {-3, &s}, {2, &s}
    };
    Intersections::sortIntersectionsAscendingTime(ints);
    EXPECT_EQ(ints[0].t, -3);
    EXPECT_EQ(ints[1].t, 2);
    EXPECT_EQ(ints[2].t, 5);
    EXPECT_EQ(ints[3].t, 7);
}

TEST(Intersections, FindHitWhenWithAllPositiveT)
{
    Sphere s{};
    Intersection i1{1, &s};
    Intersection i2{2, &s};
    Intersections xs{};
    xs.add(i2);
    xs.add(i1);
    auto i = xs.findHit();
    EXPECT_TRUE(i.isHit());
    EXPECT_EQ(i, i1);
}

TEST(Intersections, FindHitWhenWithSomeNegativeT)
{
    Sphere s{};
    Intersection i1{-1, &s};
    Intersection i2{1, &s};
    Intersections xs{};
    xs.add(i2);
    xs.add(i1);
    auto i = xs.findHit();
    EXPECT_TRUE(i.isHit());
    EXPECT_EQ(i, i2);
}

TEST(Intersections, FindHitWhenWithAllNegativeT)
{
    Sphere s{};
    Intersection i1{-2, &s};
    Intersection i2{-1, &s};
    Intersections xs{};
    xs.add(i2);
    xs.add(i1);
    auto i = xs.findHit();
    // i should be a miss
    EXPECT_FALSE(i.isHit());
}

TEST(Intersections, HitIsAlwaysLowestNonNegative)
{
    Sphere s{};
    Intersection i1{5, &s};
    Intersection i2{7, &s};
    Intersection i3{-3, &s};
    Intersection i4{2, &s};
    Intersections xs{};
    xs.add(i1);
    xs.add(i2);
    xs.add(i3);
    xs.add(i4);
    auto i = xs.findHit();
    EXPECT_TRUE(i.isHit());
    EXPECT_EQ(i, i4);
}

TEST(Intersections, AddingIntersections)
{
    // test for + operator overload with Intersections
    Sphere s{};
    Intersection i1{-3, &s};
    Intersection i2{2, &s};
    Intersection i3{5, &s};
    Intersection i4{7, &s};
    Intersections A{{i1, i2}};
    Intersections B{{i3, i4}};
    Intersections C{{i1, i2, i3, i4}};
    auto res = A + B;
    EXPECT_EQ(res.count(), 4);
    EXPECT_DOUBLE_EQ(res(0).t, -3);
    EXPECT_DOUBLE_EQ(res(1).t, 2);
    EXPECT_DOUBLE_EQ(res(2).t, 5);
    EXPECT_DOUBLE_EQ(res(3).t, 7);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Raycasting Advanced
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(Raycasting, TranslatingRays)
{
    Ray r{ Point{1, 2, 3}, Vector{0, 1, 0} };
    auto t = Transform::translation(3., 4., 5.);
    auto r2 = r.transform(t);
    EXPECT_EQ(r2.getOrigin(), Point(4, 6, 8));
    EXPECT_EQ(r2.getDirection(), Vector(0, 1, 0));
}

TEST(Raycasting, ScalingRays)
{
    Ray r{ Point{1, 2, 3}, Vector{0, 1, 0} };
    auto t = Transform::scale(2., 3., 4.);
    auto r2 = r.transform(t);
    EXPECT_EQ(r2.getOrigin(), Point(2, 6, 12));
    EXPECT_EQ(r2.getDirection(), Vector(0, 3, 0));
}

TEST(Raycasting, DefaultSphereTransformation)
{
    Sphere s{};
    const auto ident{ Matrix<double, 4>::identity() };
    EXPECT_EQ(s.getTransform(), ident);
}

TEST(Raycasting, SetTransformOfASphere)
{
    Sphere s{};
    const auto T{ Transform::translation(2., 3., 4.) };
    s.setTransform(T);
    EXPECT_EQ(s.getTransform(), T);
}

TEST(Raycasting, IntersectingScaledSphereWithRay)
{
    Sphere s{};
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    s.setTransform(Transform::scale(2., 2., 2.));
    auto xs{ s.intersect(r) };
    EXPECT_EQ(xs.count(), 2);
    EXPECT_EQ(xs(0).t, 3.);
    EXPECT_EQ(xs(1).t, 7.);
}

TEST(Raycasting, IntersectingTranslatedSphereWithRay)
{
    Sphere s{};
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    s.setTransform(Transform::translation(5., 0., 0.));
    auto xs{ s.intersect(r) };
    EXPECT_EQ(xs.count(), 0);
}

TEST(Raycasting, PrecomputingTheReflectionVector)
{
    Plane plane{};
    Ray r{ Point{0, 1, -1}, Vector{0, -HALF_SQRT_2, HALF_SQRT_2} };
    Intersection i{ HALF_SQRT_2, &plane };
    Intersections xs{ i };
    IntersectionState iState{ i, r, xs };
    EXPECT_EQ(iState.vReflect, (Vector{ 0, HALF_SQRT_2, HALF_SQRT_2}));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Refractive Ray Intersections
////////////////////////////////////////////////////////////////////////////////////////////////////
class RefractiveIntersections: public ::testing::Test
{
  protected:
    Sphere a;
    Sphere b;
    Sphere c;
    Intersections xs;
    Intersection i0{2, &a}, i1{2.75, &b}, i2{3.25, &c},
                 i3{4.75, &b}, i4{5.25, &c}, i5{6, &a};
    Ray r{ Point{0, 0, -4}, Vector{0, 0, 1} };

    void SetUp() override {
        a = Sphere::glassySphere();
        a.setTransform(Transform::scale(2., 2., 2.));
        b = Sphere::glassySphere();
        b.setTransform(Transform::translation(0., 0., -0.25));
        b.setRefraction(1.0, 2.0);
        c = Sphere::glassySphere();
        c.setTransform(Transform::translation(0., 0., 0.25));
        c.setRefraction(1.0, 2.5);
        xs.add(i0);
        xs.add(i1);
        xs.add(i2);
        xs.add(i3);
        xs.add(i4);
        xs.add(i5);
    }
};

TEST_F(RefractiveIntersections, PrecomputingRefractiveIndices_0)
{
    // n1 and n2 (the refractive indices at either side of a ray-object intersection) are
    //  computed properly at six different points of intersection, between three overlaid
    //  and glassy refractive spheres
    auto iState = IntersectionState{ i0, r, xs };
    EXPECT_EQ(iState.n1, 1.0);
    EXPECT_EQ(iState.n2, 1.5);
}

TEST_F(RefractiveIntersections, PrecomputingRefractiveIndices_1)
{
    auto iState = IntersectionState{ i1, r, xs };
    EXPECT_EQ(iState.n1, 1.5);
    EXPECT_EQ(iState.n2, 2.0);
}

TEST_F(RefractiveIntersections, PrecomputingRefractiveIndices_2)
{
    auto iState = IntersectionState{ i2, r, xs };
    EXPECT_EQ(iState.n1, 2.0);
    EXPECT_EQ(iState.n2, 2.5);
}

TEST_F(RefractiveIntersections, PrecomputingRefractiveIndices_3)
{
    auto iState = IntersectionState{ i3, r, xs };
    EXPECT_EQ(iState.n1, 2.5);
    EXPECT_EQ(iState.n2, 2.5);
}

TEST_F(RefractiveIntersections, PrecomputingRefractiveIndices_4)
{
    auto iState = IntersectionState{ i4, r, xs };
    EXPECT_EQ(iState.n1, 2.5);
    EXPECT_EQ(iState.n2, 1.5);
}

TEST_F(RefractiveIntersections, PrecomputingRefractiveIndices_5)
{
    auto iState = IntersectionState{ i5, r, xs };
    EXPECT_EQ(iState.n1, 1.5);
    EXPECT_EQ(iState.n2, 1.0);
}

TEST_F(RefractiveIntersections, UnderPointIsCalculated)
{
    // a ray intersects a glass sphere at z=0 and an under point at just over z=0
    //  is generated
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    Sphere s = Sphere::glassySphere();
    s.setTransform(Transform::translation(0., 0., 1.));
    Intersection i{ 5, &s };
    Intersections xs{ i };
    IntersectionState iState{ i, r, xs };
    EXPECT_TRUE(iState.pointBelowSurface.z > EPSILON / 2.);
    EXPECT_TRUE(iState.point.z < iState.pointBelowSurface.z);
}
