#include "gtest/gtest.h"
#include "raytracer/renderer/colour.hpp"

using namespace rt;

class BasicColours : public ::testing::Test
{
  protected:
    Colour c1{ .9, .6, .75 };
    Colour c2{ .7, .1, .25 };
};

TEST_F(BasicColours, DefaultIsBlack)
{
    auto c = Colour{ };
    auto expected = Colour{0., 0., 0.};
    EXPECT_EQ(c, expected);
}

TEST_F(BasicColours, AddingColours)
{
    auto sum = c1 + c2;
    auto expected = Colour{ 1.6, .7, 1.0 };
    EXPECT_EQ(sum, expected);
}

TEST_F(BasicColours, SubtractingColours)
{
    auto sub = c1 - c2;
    auto expected = Colour{ .2, .5, .5 };
    EXPECT_EQ(sub, expected);
}

TEST_F(BasicColours, MultiplyByScalar)
{
    auto c = Colour{ .2, .3, .4 };
    auto expected = Colour{ .4, .6, .8 };
    EXPECT_EQ(c * 2, expected);
}

TEST_F(BasicColours, MultiplyingColours)
{
//    auto product = c1 * c2;
    auto expected = Colour{ .63, .06, .1875 };
    EXPECT_EQ(c1 * c2, expected);
}

TEST_F(BasicColours, ColourToPPM8Bit)
{
    auto c = Colour{ .63, .06, .1875 };
    auto expected = "161 15 48";
    auto res = Colour::toPPM8b(c);
    EXPECT_EQ(expected, res);
}

TEST_F(BasicColours, PPM8BitIsClampedTo255)
{
    auto c = Colour{ 2.0, -2.0, .2 };
    auto expected = "255 0 51";
    auto res = Colour::toPPM8b(c);
    EXPECT_EQ(expected, res);
}

TEST_F(BasicColours, RGBToPPMInt)
{
    // rgb to ppm integer conversion
    auto c = Colour{ 2.0, -2.0, .2 };
//    static unsigned int rgbToPPM(const double rgb, const unsigned int maxVal=255);
    auto r = Colour::rgbToPPM(c.R);
    auto g = Colour::rgbToPPM(c.G);
    auto b = Colour::rgbToPPM(c.B);
    EXPECT_EQ(r, 255);
    EXPECT_EQ(g, 0);
    EXPECT_EQ(b, 51);
}

