#include "raytracer/intersection.h"
#include "raytracer/shapes.h"
#include "raytracer/sphere.h"
#include "raytracer/matrix.h"
#include "raytracer/rays.h"
#include "gtest/gtest.h"

#include <numbers>

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Calculating Normals to a Sphere
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(SphereNormals, NormalAtPointOnXAxis)
{
    Sphere s{};
    auto n = s.normalAt(Point{1, 0, 0});
    EXPECT_EQ(n, Vector(1, 0, 0));
}

TEST(SphereNormals, NormalAtPointOnYAxis)
{
    Sphere s{};
    auto n = s.normalAt(Point{0, 1, 0});
    EXPECT_EQ(n, Vector(0, 1, 0));
}

TEST(SphereNormals, NormalAtPointOnZAxis)
{
    Sphere s{};
    auto n = s.normalAt(Point{0, 0, 1});
    EXPECT_EQ(n, Vector(0, 0, 1));
}

TEST(SphereNormals, NormalAtNonAxialPoint)
{
    using namespace std::numbers;
    Sphere s{};
    auto n = s.normalAt(Point{sqrt3/3., sqrt3/3., sqrt3/3.});
    EXPECT_EQ(n, Vector(sqrt3/3., sqrt3/3., sqrt3/3.));
}

TEST(SphereNormals, IsNormalizedVector)
{
    using namespace std::numbers;
    Sphere s{};
    auto n = s.normalAt(Point{sqrt3/3., sqrt3/3., sqrt3/3.});
    EXPECT_EQ(n, n.normalize());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sphere Materials
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(SphereMaterials, SphereHasDefaultMaterial)
{
    Sphere s{};
    Material m{s.getMaterial()};
    EXPECT_EQ(m, Material());
}

TEST(SphereMaterials, AssigningMaterialToSphere)
{
    Sphere s{};
    Material m{};
    m.ambient = 1.0;
    s.setMaterial(m);
    EXPECT_EQ(m, s.getMaterial());
}

TEST(SphereMaterials, GlassSphereHelper)
{
    auto glassy = Sphere::glassySphere();
    EXPECT_DOUBLE_EQ(glassy.getMaterial().transparency, 1.0);
    EXPECT_DOUBLE_EQ(glassy.getMaterial().refraction, 1.5);
}