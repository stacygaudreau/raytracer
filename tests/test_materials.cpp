#include "materials.h"
#include "colours.h"
#include "lighting.h"
#include "utils.h"
#include "sphere.h"
#include "gtest/gtest.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Material Basics
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(MaterialsBasics, DefaultMaterialConstructed)
{
    // the default material is constructed with default values
    Material m{};
    EXPECT_EQ(m.colour, Colour(1, 1, 1));
    EXPECT_DOUBLE_EQ(m.ambient, 0.1);
    EXPECT_DOUBLE_EQ(m.diffuse, 0.9);
    EXPECT_DOUBLE_EQ(m.specular, 0.9);
    EXPECT_DOUBLE_EQ(m.shininess, 200.0);
    EXPECT_DOUBLE_EQ(m.reflectivity, 0.0);
    EXPECT_DOUBLE_EQ(m.transparency, 0.0);
    EXPECT_DOUBLE_EQ(m.refraction, 1.0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Lighting Basic Materials
////////////////////////////////////////////////////////////////////////////////////////////////////
class LightingBasicMaterials: public ::testing::Test
{
  protected:
    Material m{};
    Point position{};
    Colour WHITE{1, 1, 1};
    Sphere s{};
};


TEST_F(LightingBasicMaterials, EyeDirectlyBetweenLightAndSurface)
{
    Vector eye{0, 0, -1};
    Vector normal{0, 0, -1};
    PointLight light{Point{0, 0, -10}, WHITE};
    auto res = m.lightPixel(light, position, s.transformPoint(position), eye, normal);
    EXPECT_EQ(res, Colour(1.9, 1.9, 1.9));
}

TEST_F(LightingBasicMaterials, Eye45DegBetweenLightAndSurface)
{
    Vector eye{0, SQRT_2/2., -SQRT_2/2.};
    Vector normal{0, 0, -1};
    PointLight light{Point{0, 0, -10}, WHITE};
    auto res = m.lightPixel(light, position, s.transformPoint(position), eye, normal);
    EXPECT_EQ(res, Colour(1.0, 1.0, 1.0));
}

TEST_F(LightingBasicMaterials, Eye45DegOppositeSurface)
{
    Vector eye{0, 0, -1};
    Vector normal{0, 0, -1};
    PointLight light{Point{0, 10, -10}, WHITE};
    auto res = m.lightPixel(light, position, s.transformPoint(position), eye, normal);
    EXPECT_EQ(res, Colour(0.7364, 0.7364, 0.7364));
}

TEST_F(LightingBasicMaterials, EyeInReflectionPathDirectly)
{
    Vector eye{0, -SQRT_2/2., -SQRT_2/2.};
    Vector normal{0, 0, -1};
    PointLight light{Point{0, 10, -10}, WHITE};
    auto res = m.lightPixel(light, position, s.transformPoint(position), eye, normal);
    EXPECT_EQ(res, Colour(1.6364, 1.6364, 1.6364));
}

TEST_F(LightingBasicMaterials, LightOccludedBehindSurface)
{
    // should be the intensity of only the ambient light component
    Vector eye{0, 0, -1};
    Vector normal{0, 0, -1};
    PointLight light{Point{0, 0, 10}, WHITE};
    auto res = m.lightPixel(light, position, s.transformPoint(position), eye, normal);
    EXPECT_EQ(res, Colour(0.1, 0.1, 0.1));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Basic Shadows
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_F(LightingBasicMaterials, LightWithTheSurfaceInShadow)
{
    // binary shadow only uses ambient component when point is in shadow
    auto eye = Vector{ 0, 0, -1 };
    auto normal = Vector{  0, 0, -1 };
    auto light = PointLight(Point{ 0, 0, -10 }, { 1.0, 1.0, 1.0 });
    bool inShadow{ true };
    auto res = m.lightPixel(light, position, s.transformPoint(position),
                            eye, normal, inShadow);
    EXPECT_EQ(res, (Colour{ 0.1, 0.1, 0.1 }));
}


