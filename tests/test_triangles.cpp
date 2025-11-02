#include "gtest/gtest.h"
#include "raytracer/triangle.h"
#include "raytracer/world.h"
#include "raytracer/rays.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Triangles
////////////////////////////////////////////////////////////////////////////////////////////////////
class Triangles: public ::testing::Test
{
  protected:
    Point p1{0, 1, 0}, p2{-1, 0, 0}, p3{1, 0, 0};
    Triangle t{ p1, p2, p3 };
};

TEST_F(Triangles, ConstructedWithTwoEdgeVectors)
{
    // two edge vectors and normal are precomputed during construction
    EXPECT_EQ(t.getP1(), p1);
    EXPECT_EQ(t.getP2(), p2);
    EXPECT_EQ(t.getP3(), p3);
    EXPECT_EQ(t.getEdge1(), Vector(-1, -1, 0));
    EXPECT_EQ(t.getEdge2(), Vector(1, -1, 0));
    EXPECT_EQ(t.getNormal(), Vector(0, 0, -1));
}

TEST_F(Triangles, SameNormalAcrossSurface)
{
    // normal vector is the same across the surface
    auto n1 = t.localNormalAt(Point{ 0, 0.5, 0 });
    auto n2 = t.localNormalAt(Point{ -0.5, 0.75, 0 });
    auto n3 = t.localNormalAt(Point{ 0.5, 0.25, 0 });
    EXPECT_EQ(n1, t.getNormal());
    EXPECT_EQ(n2, t.getNormal());
    EXPECT_EQ(n3, t.getNormal());
}

TEST_F(Triangles, RayParallelToTriangleMisses)
{
    // a parallel ray should not intersect the triangle
    Ray r{ Point{0, -1, -2}, Vector{0, 1, 0} };
    auto xs = t.localIntersect(r);
    EXPECT_TRUE(xs.isEmpty());
}

TEST_F(Triangles, RayMissesEdge_P1P3)
{
    // ray misses triangle over the p1-p3 edge
    Ray r{ Point{1, 1, -2}, Vector{0, 0, 1} };
    auto xs = t.localIntersect(r);
    EXPECT_TRUE(xs.isEmpty());
}

TEST_F(Triangles, RayMissesEdge_P1P2)
{
    Ray r{ Point{-1, 1, -2}, Vector{0, 0, 1} };
    auto xs = t.localIntersect(r);
    EXPECT_TRUE(xs.isEmpty());
}

TEST_F(Triangles, RayMissesEdge_P2P3)
{
    Ray r{ Point{0, -1, -2}, Vector{0, 0, 1} };
    auto xs = t.localIntersect(r);
    EXPECT_TRUE(xs.isEmpty());
}

TEST_F(Triangles, RayDoesIntersect)
{
    Ray r{ Point{0, 0.5, -2}, Vector{0, 0, 1} };
    auto xs = t.localIntersect(r);
    EXPECT_EQ(xs.count(), 1);
    EXPECT_EQ(xs(0).t, 2);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Smooth Triangles
////////////////////////////////////////////////////////////////////////////////////////////////////
class SmoothTriangles: public ::testing::Test
{
  protected:
    Point p1{ 0, 1, 0 };
    Point p2{ -1, 0, 0 };
    Point p3{ 1, 0, 0 };
    Vector n1{ 0, 1, 0 };
    Vector n2{ -1, 0, 0 };
    Vector n3{ 1, 0, 0 };
    SmoothTriangle t{ p1, p2, p3, n1, n2, n3 };
};

TEST_F(SmoothTriangles, Constructor)
{
    EXPECT_EQ(t.getP1(), p1);
    EXPECT_EQ(t.getP2(), p2);
    EXPECT_EQ(t.getP3(), p3);
    EXPECT_EQ(t.getN1(), n1);
    EXPECT_EQ(t.getN2(), n2);
    EXPECT_EQ(t.getN3(), n3);
}

TEST_F(SmoothTriangles, IntersectionHasUV)
{
    // u and v components are added to the intersection class
    // these store where on the triangle face the intersection occured
    Triangle t{ p1, p2, p3 };
    Intersection i{ 3.5, &t, 0.2, 0.4 };
    EXPECT_DOUBLE_EQ(i.u, 0.2);
    EXPECT_DOUBLE_EQ(i.v, 0.4);
}

TEST_F(SmoothTriangles, AnIntersectionStoresUV)
{
    // smooth triangle ray-intersection stores UV
    Triangle t{ p1, p2, p3 };
    Ray r{ Point{ -0.2, 0.3, -2 }, Vector{ 0, 0, 1 } };
    Intersections xs = t.localIntersect(r);
    EXPECT_DOUBLE_EQ(xs(0).u, 0.45);
    EXPECT_DOUBLE_EQ(xs(0).v, 0.25);
}

TEST_F(SmoothTriangles, NormalIsInterpolatedFromUV)
{
    // the u, v components are used to interpolate a value
    //  for the triangle's normal at any given point on its surface
    /// this is how smoothing is achieved.
    Intersection i{ 1.0, &t, 0.45, 0.25 };
    auto n = t.normalAt(Point{}, i);
    EXPECT_EQ(n, Vector(-0.5547, 0.83205, 0));
}

TEST_F(SmoothTriangles, NormalIsPreparedByIntersectionState)
{
    Intersection i{ 1.0, &t, 0.45, 0.25 };
    Ray r{ Point{-0.2, 0.3, -2}, Vector{0, 0, 1} };
    auto xs = Intersections{ i };
    auto istate = IntersectionState{ i, r, xs };
    EXPECT_EQ(istate.normal, Vector(-0.5547, 0.83205, 0));
}





