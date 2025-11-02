#include "gtest/gtest.h"
#include "raytracer/tuples.h"

#include <numbers>

using namespace rt;

// Tuple Basics
TEST(TupleBasics, ZeroWIsPoint)
{
    // a tuple created with w=1.0 is a point
    Tuple a{ 4.3, -4.2, 3.1, 1.0 };
    EXPECT_DOUBLE_EQ(a.x, 4.3);
    EXPECT_DOUBLE_EQ(a.y, -4.2);
    EXPECT_DOUBLE_EQ(a.z, 3.1);
    EXPECT_DOUBLE_EQ(a.w, 1.0);
    EXPECT_TRUE(a.isPoint());
    EXPECT_FALSE(a.isVector());
}
TEST(TupleBasics, OneWIsVector)
{
    // a tuple created with w=0.0 is a vector
    Tuple a{ 4.3, -4.2, 3.1, 0.0 };
    EXPECT_DOUBLE_EQ(a.x, 4.3);
    EXPECT_DOUBLE_EQ(a.y, -4.2);
    EXPECT_DOUBLE_EQ(a.z, 3.1);
    EXPECT_DOUBLE_EQ(a.w, 0.0);
    EXPECT_FALSE(a.isPoint());
    EXPECT_TRUE(a.isVector());
}

TEST(TupleBasics, PointCreationWIs1)
{
    // point() creates tuples with w=1
    auto p = Point(4, -4, 3);
    auto t = Tuple{ 4, -4, 3, 1 };
    EXPECT_EQ(p, t);
}

TEST(TupleBasics, VectorCreationWis0)
{
    // vector() creates tuples with w=0
    auto p = Point(4, -4, 3);
    auto t = Tuple{ 4, -4, 3, 1 };
    EXPECT_EQ(p, t);
}

TEST(VectorGeometry, Normalisation)
{
    // normalising vector (4, 0, 0) yields (1, 0, 0)
    auto v = Vector(4, 0, 0);
    EXPECT_EQ(v.normalize(), Vector(1, 0, 0));
}

TEST(VectorGeometry, DotProduct)
{
    // dot product of two tuples is 20
    auto a = Vector(1, 2, 3);
    auto b = Vector(2, 3, 4);
    EXPECT_DOUBLE_EQ(Tuple::dot(a, b), 20);
}

TEST(VectorGeometry, CrossProduct)
{
    // cross product of two vectors
    auto a = Vector(1, 2, 3);
    auto b = Vector(2, 3, 4);
    ASSERT_EQ(cross(a, b), Vector(-1, 2, -1));
    ASSERT_EQ(cross(b, a), Vector(1, -2, 1));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Reflecting Vectors off surfaces
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(VectorReflections, ReflectVectorAt45Deg)
{
    Vector v{1, -1, 0};
    Tuple n{Vector{0, 1, 0}};
    auto r = Vector::reflect(v, n);
    EXPECT_EQ(r, Vector(1, 1, 0));
}

TEST(VectorReflections, ReflectVectorOffSlantedSurface)
{
    using namespace std::numbers;
    Vector v{0, -1, 0};
    Tuple n{Vector{sqrt2/2., sqrt2/2., 0.}};
    auto r = Vector::reflect(v, n);
    EXPECT_EQ(r, Vector(1, 0, 0));
}

TEST(VectorReflections, ClassMethodVersion)
{
    using namespace std::numbers;
    Vector v{0, -1, 0};
    Tuple n{Vector{sqrt2/2., sqrt2/2., 0.}};
    auto r = v.reflect(n);
    EXPECT_EQ(r, Vector(1, 0, 0));
}

// Tuple Arithmetic
//#define CATCH_CONFIG_MAIN
//
//#include "catch.hpp"
//#include "Tuples/Tuples.h"
//
//
//TEST_CASE("Tuple Basics")
//{
//    SECTION("a tuple created with w=1.0 is a point")
//    {
//        Tuple a{ 4.3, -4.2, 3.1, 1.0 };
//        REQUIRE(a.x, 4.3));
//        EXPECT_DOUBLE_EQ(a.y, -4.2));
//        EXPECT_DOUBLE_EQ(a.z, 3.1));
//        EXPECT_DOUBLE_EQ(a.w, 1.0));
//        REQUIRE(a.isPoint() == true);
//        REQUIRE(a.isVector() == false);
//    }
//    SECTION("a tuple created with w=0.0 is a vector")
//    {
//        Tuple a{ 4.3, -4.2, 3.1, 0.0 };
//        EXPECT_DOUBLE_EQ(a.x, 4.3));
//        EXPECT_DOUBLE_EQ(a.y, -4.2));
//        EXPECT_DOUBLE_EQ(a.z, 3.1));
//        EXPECT_DOUBLE_EQ(a.w, 0.0));
//        REQUIRE(a.isPoint() == false);
//        REQUIRE(a.isVector() == true);
//    }
//    SECTION("point() creates tuples with w=1")
//    {
//        auto p = Point(4, -4, 3);
//        auto t = Tuple{ 4, -4, 3, 1 };
//        EXPECT_DOUBLE_EQ(p == t);
//    }
//    SECTION("vector() creates tuples with w=0")
//    {
//        auto p = Vector(4, -4, 3);
//        auto t = Tuple{ 4, -4, 3, 0 };
//        EXPECT_DOUBLE_EQ(p == t);
//    }
//}
//

TEST(TupleArithmetic, AddingPointAndVector)
{
    Tuple p{3, -2, 5, 1};
    Tuple v{-2, 3, 1, 0};
    Tuple expected{1, 1, 6, 1};
    EXPECT_EQ(p + v, expected);
}

TEST(TupleArithmetic, MultiplyTupleByScalar)
{
    Tuple a{1, -2, 3, -4};
    Tuple expected{3.5, -7, 10.5, -14};
    EXPECT_EQ(a * 3.5, expected);
}
//    SECTION("multiplying a tuple by a scalar")
//    {
//        auto a = Tuple{ 1, -2, 3, -4 };
//        // auto v2 = Vector(5, 6, 7);
//        REQUIRE(a * 3.5 == Tuple(3.5, -7, 10.5, -14));
//    }

//TEST_CASE("Tuple Arithmetic")
//{
//    SECTION("subtracting two points results in a vector")
//    {
//        auto p1 = Point(3, 2, 1);
//        auto p2 = Point(5, 6, 7);
//        REQUIRE(p1 - p2 == Vector(-2, -4, -6));
//    }
//    SECTION("subtracting a vector from a point results in a point")
//    {
//        auto p1 = Point(3, 2, 1);
//        auto p2 = Vector(5, 6, 7);
//        REQUIRE(p1 - p2 == Point(-2, -4, -6));
//    }
//    SECTION("subtracting two vectors results in a vector")
//    {
//        auto v1 = Vector(3, 2, 1);
//        auto v2 = Vector(5, 6, 7);
//        REQUIRE(v1 - v2 == Vector(-2, -4, -6));
//    }
//    SECTION("negating a tuple")
//    {
//        auto v1 = Tuple{ 1, -2, 3, -4 };
//        REQUIRE(-v1 == Tuple{ -1, 2, -3, 4 });
//    }
//    SECTION("multiplying a tuple by a fraction")
//    {
//        auto a = Tuple{ 1, -2, 3, -4 };
//        // auto v2 = Vector(5, 6, 7);
//        REQUIRE(a * .5 == Tuple(0.5, -1, 1.5, -2));
//    }
//    SECTION("multiplying a tuple by a scalar")
//    {
//        auto a = Tuple{ 1, -2, 3, -4 };
//        // auto v2 = Vector(5, 6, 7);
//        REQUIRE(a / 2 == Tuple(0.5, -1, 1.5, -2));
//    }
//}
//
//TEST_CASE("Vector Magnitude")
//{
//    SECTION("magnitude of vector (1, 0, 0)")
//    {
//        auto v = Vector(1, 0, 0);
//        REQUIRE(v.magnitude() == 1);
//    }
//    SECTION("magnitude of vector (0, 1, 0)")
//    {
//        auto v = Vector(0, 1, 0);
//        REQUIRE(v.magnitude() == 1);
//    }
//    SECTION("magnitude of vector (0, 0, 1)")
//    {
//        auto v = Vector(0, 0, 1);
//        REQUIRE(v.magnitude() == 1);
//    }
//    SECTION("magnitude of vector (1, 2, 3)")
//    {
//        auto v = Vector(1, 2, 3);
//        REQUIRE(v.magnitude() == sqrt(14));
//    }
//    SECTION("magnitude of vector (-1, -2, -3)")
//    {
//        auto v = Vector(-1, -2, -3);
//        REQUIRE(v.magnitude() == sqrt(14));
//    }
//}
//TEST_CASE("Normalising Vectors")
//{
//    SECTION("normalising vector (4, 0, 0) yields (1, 0, 0)")
//    {
//        auto v = Vector(4, 0, 0);
//        REQUIRE(Tuple::normalize(v) == Vector(1, 0, 0));
//    }
//}
