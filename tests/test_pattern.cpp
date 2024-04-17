#include "gtest/gtest.h"
#include "patterns.h"
#include "colours.h"
#include "materials.h"
#include "sphere.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// StripedPattern
////////////////////////////////////////////////////////////////////////////////////////////////////
class StripedPatternBasics: public ::testing::Test
{
  protected:
    Colour black{ 0, 0, 0 };
    Colour white{ 1, 1, 1 };
    StripedPattern stripes{ white, black };
    Sphere s{};
};

TEST_F(StripedPatternBasics, StripePatternIsConstructed)
{
    EXPECT_EQ(stripes.a, white);
    EXPECT_EQ(stripes.b, black);
}

TEST_F(StripedPatternBasics, YColourOfStripedPatternIsConstant)
{
    EXPECT_EQ(stripes.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(stripes.colourAt(Point{0, 1, 0}), white);
    EXPECT_EQ(stripes.colourAt(Point{0, 2, 0}), white);
}

TEST_F(StripedPatternBasics, ZColourOfStripedPatternIsConstant)
{
    EXPECT_EQ(stripes.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(stripes.colourAt(Point{0, 0, 1}), white);
    EXPECT_EQ(stripes.colourAt(Point{0, 0, 2}), white);
}

TEST_F(StripedPatternBasics, XColourOfStripedPatternAlternates)
{
    EXPECT_EQ(stripes.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(stripes.colourAt(Point{0.9, 0, 0}), white);
    EXPECT_EQ(stripes.colourAt(Point{1, 0, 0}), black);
    EXPECT_EQ(stripes.colourAt(Point{-0.1, 0, 0}), black);
    EXPECT_EQ(stripes.colourAt(Point{-1, 0, 0}), black);
    EXPECT_EQ(stripes.colourAt(Point{-1.1, 0, 0}), white);
}

TEST_F(StripedPatternBasics, StripesWithObjectTransformation)
{
    s.setTransform(Transform::scale(2., 2., 2.));
    const Point pWorld{ 1.5, 0, 0 };
    Colour c = stripes.colourAtShape(s.transformPoint(pWorld));
    EXPECT_EQ(c, white);
}

TEST_F(StripedPatternBasics, StripesWithPatternTransformation)
{
    stripes.setTransform(Transform::scale(2.0, 2.0, 2.0));
    const Point pWorld{ 1.5, 0, 0 };
    Colour c = stripes.colourAtShape(s.transformPoint(pWorld));
    EXPECT_EQ(c, white);
}

TEST_F(StripedPatternBasics, StripesWithBothObjectAndPatternTransform)
{
    s.setTransform(Transform::scale(2., 2., 2.));
    stripes.setTransform(Transform::translation(0.5, 0.0, 0.0));
    const Point pWorld{ 2.5, 0, 0 };
    Colour c = stripes.colourAtShape(s.transformPoint(pWorld));
    EXPECT_EQ(c, white);
}

TEST_F(StripedPatternBasics, LightingWithStripesApplied)
{
    Material m{};

    m.setPattern(&stripes);
    m.ambient = 1.0;
    m.diffuse = 0.0;
    m.specular = 0.0;
    auto vEye = Vector{0, 0, -1};
    auto vNormal = Vector{0, 0, -1};
    PointLight light{ Point{0, 0, -10}, Colour{1, 1, 1} };
    const Point p1{0.9, 0, 0};
    Colour c1 = m.lightPixel(light, p1, s.transformPoint(p1), vEye, vNormal);
    const Point p2{1.1, 0, 0};
    Colour c2 = m.lightPixel(light, p2, s.transformPoint(p2), vEye, vNormal);
    EXPECT_TRUE(m.hasPattern());
    EXPECT_EQ(c1, Colour(1, 1, 1));
    EXPECT_EQ(c2, Colour(0, 0, 0));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  BasePatterns
////////////////////////////////////////////////////////////////////////////////////////////////////
class BasePatternClass: public ::testing::Test
{
  protected:

    class TestPattern: public Pattern
    {
      public:
        TestPattern(Colour a, Colour b): Pattern(a, b) {};

        virtual Colour colourAt(Tuple point) override
        {
            std::cout << point << "\n";
            return { point.x, point.y, point.z };
        };
    };

    Colour white{ 1, 1, 1 };
    Colour black{ 0, 0, 0 };
    TestPattern pattern{ white, black };
};


TEST_F(BasePatternClass, DefaultTransformationIsIdentity)
{
    EXPECT_EQ(pattern.getTransform(), TransformationMatrix::identity());
    EXPECT_EQ(pattern.getInverseTransform(), TransformationMatrix::identity().inverse());
}

TEST_F(BasePatternClass, TransformationIsSet)
{
    const auto T = Transform::translation(1., 2., 3.);
    pattern.setTransform(T);
    EXPECT_EQ(pattern.getTransform(), T);
    EXPECT_EQ(pattern.getInverseTransform(), T.inverse());
}

TEST_F(BasePatternClass, ObjectTransformationIsApplied)
{
    Sphere s{};
    s.setTransform(Transform::scale(2., 2., 2.));
    Colour c = pattern.colourAtShape(s.transformPoint(Point{ 2, 3, 4 }));
    EXPECT_EQ(c, (Colour{ 1, 1.5, 2 }));
}

TEST_F(BasePatternClass, PatternTransformIsApplied)
{
    Sphere s{};
    pattern.setTransform(Transform::scale(2., 2., 2.));
    Colour c = pattern.colourAtShape(s.transformPoint(Point{ 2, 3, 4 }));
    EXPECT_EQ(c, (Colour{ 1, 1.5, 2 }));
}

 // pretty sure there is errata in the book tests for this one.
TEST_F(BasePatternClass, ObjectAndPatternTransformIsApplied)
{
    Sphere s{};
    s.setTransform(Transform::scale(2., 2., 2.));
    pattern.setTransform(Transform::translation(0.5, 1.0, 1.5));
    Colour c = pattern.colourAtShape(s.transformPoint(Point{ 2.5, 3, 3.5 }));
    EXPECT_EQ(c, (Colour{ 0.75, 0.5, 0.25 }));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Miscellaneous Patterns
////////////////////////////////////////////////////////////////////////////////////////////////////
class MiscPatterns: public ::testing::Test
{
  protected:
    Colour white{ 1, 1, 1 };
    Colour black{ 0, 0, 0 };
};


TEST_F(MiscPatterns, GradientLerpsBetweenColours)
{
    GradientPattern grad{ white, black };
    EXPECT_EQ(grad.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(grad.colourAt(Point{0.25, 0, 0}), Colour(0.75, 0.75, 0.75));
    EXPECT_EQ(grad.colourAt(Point{0.5, 0, 0}), Colour(0.5, 0.5, 0.5));
    EXPECT_EQ(grad.colourAt(Point{0.75, 0, 0}), Colour(0.25, 0.25, 0.25));
}

TEST_F(MiscPatterns, RingExtendsInBothXAndZ)
{
    RingPattern ring{ white, black };
    EXPECT_EQ(ring.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(ring.colourAt(Point{1, 0, 0}), black);
    EXPECT_EQ(ring.colourAt(Point{0, 0, 1}), black);
    EXPECT_EQ(ring.colourAt(Point{0.708, 0, 0.708}), black);
}

TEST_F(MiscPatterns, CheckersRepeatInX)
{
    CheckersPattern checkers{ white, black };
    EXPECT_EQ(checkers.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(checkers.colourAt(Point{0.99, 0, 0}), white);
    EXPECT_EQ(checkers.colourAt(Point{1.01, 0, 0.0}), black);
}

TEST_F(MiscPatterns, CheckersRepeatInY)
{
    CheckersPattern checkers{ white, black };
    EXPECT_EQ(checkers.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(checkers.colourAt(Point{0, 0.99, 0}), white);
    EXPECT_EQ(checkers.colourAt(Point{0, 1.01, 0.0}), black);
}

TEST_F(MiscPatterns, CheckersRepeatInZ)
{
    CheckersPattern checkers{ white, black };
    EXPECT_EQ(checkers.colourAt(Point{0, 0, 0}), white);
    EXPECT_EQ(checkers.colourAt(Point{0, 0, 0.99}), white);
    EXPECT_EQ(checkers.colourAt(Point{0, 0, 1.01}), black);
}




