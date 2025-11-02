#include "gtest/gtest.h"
#include "raytracer/colours.h"
#include "raytracer/textures.h"
#include "raytracer/materials.h"
#include "raytracer/sphere.h"
#include "raytracer/tuples.h"
#include "raytracer/plane.h"

using namespace rt;

// todo: re-write these tests for Texture::Generative

////////////////////////////////////////////////////////////////////////////////////////////////////
// Textures (generative)
////////////////////////////////////////////////////////////////////////////////////////////////////
//class GenerativeTextures: public ::testing::Test
//{
//  protected:
//    Colour black{ 0, 0, 0 };
//    Colour white{ 1, 1, 1 };
//    StripedPattern stripes{ white, black };
//    Sphere s{};
//
//    class TestTexture: public Texture::Generative
//    {
//      public:
//        TestTexture() { }
//
//        Tuple applyToNormal(Tuple normal, Tuple point) override
//        {
//            Vector perturb{ 0.1*A.x, 0.1*A.y, 0.1*A.z };
//            return normal + perturb;
//        }
//    };
//};
//
//TEST_F(GenerativeTextures, TextureIsConstructed)
//{
//    // a generative texture is constructed with default amounts
//    //  and positioning
//    TestTexture t{};
//    EXPECT_EQ(t.xAmt, 1.0);
//    EXPECT_EQ(t.yAmt, 1.0);
//    EXPECT_EQ(t.zAmt, 1.0);
//}
//
//TEST_F(GenerativeTextures, TextureIsAddedToMaterial)
//{
//    // a texture can be added to a Material
//    TestTexture t{};
//    Material m{};
//    EXPECT_FALSE(m.hasTexture());
//    m.setTexture(&t);
//    EXPECT_TRUE(m.hasTexture());
//    EXPECT_EQ(m.getTexture(), &t);
//}
//
//TEST_F(GenerativeTextures, TextureAppliesToSurfaceNormal)
//{
//    // a test texture applies default amounts to the surface normal
//    TestTexture t{};
//    Material m{};
//    m.setTexture(&t);
//    auto res = t.applyToNormal(Vector{0., 0., 0.}, Point{0, 0, 0});
//    EXPECT_EQ(res, Vector( 0.1, 0.1, 0.1 ));
//    // we can also modify the amount of the texture's perturbation in x, y and z
//    t.xAmt = 0.25;
//    t.yAmt = 0.5;
//    t.zAmt = 2.0;
//    res = t.applyToNormal(Vector{1., 1., 1.}, Point{0, 0, 0});
//    EXPECT_EQ(Vector(1.025, 1.05, 1.2), res);
//}
//
//TEST_F(GenerativeTextures, ShapeNormalIsPerturbed)
//{
//    // the normal of a Shape is perturbed when there is a Texture present on
//    // its Material
//    Plane s1{};
//    TestTexture t{};
//    Material m{};
//    m.setTexture(&t);
//    s1.setMaterial(m);
//    auto perturbed = s1.normalAt(Point{0., 0., 0.}); // usually this would be { 0.0, 1.0, 0.0 }
//    EXPECT_EQ(perturbed, Vector( 0.1, 1.1, 0.1 ));
//}
//
//TEST_F(GenerativeTextures, WavesAppliesInOneDimension)
//{
//    // a sinusoidal surface pattern is applied in one dimension
//    Plane s1{};
//    Texture::Waves waves{};
//    Material m{};
//    m.setTexture(&waves);
//    s1.setMaterial(m);
//    // by default the wavy texture, dependent on y, modulates the y component
//    // of the surface normal
//    auto perturbed = s1.normalAt(Point{-0.5, 0.0, 0.5});
//    EXPECT_EQ(perturbed, Vector( 0.0, 1.5, 0.0 ));
//    perturbed = s1.normalAt(Point{-0.5, 0.25, 0.5});
//    EXPECT_EQ(perturbed, Vector( 0.0, 2.0, 0.0 ));
//    perturbed = s1.normalAt(Point{-0.5, 0.75, 0.5});
//    EXPECT_EQ(perturbed, Vector( 0.0, 1.0, 0.0 ));
//}
