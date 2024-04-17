#include "gtest/gtest.h"
#include <matrix_2d.h>
#include <colours.h>

using namespace rt;

class Matrix2DBasics : public ::testing::Test
{
  protected:
    Matrix2D<Colour> pixels = Matrix2D<Colour>(10, 20);
};

TEST_F(Matrix2DBasics, CreatesPixelMatrix)
{
    auto black = Colour{ };
    EXPECT_EQ(pixels.get(0, 0), black);
}

TEST_F(Matrix2DBasics, BasicGeometry)
{
    EXPECT_EQ(pixels.getWidth(), 10);
    EXPECT_EQ(pixels.getHeight(), 20);
}

TEST_F(Matrix2DBasics, AllElementsInitAsZero)
{
    EXPECT_EQ(pixels.get(0, 0), Colour{});
    EXPECT_EQ(pixels.get(pixels.getWidth()-1, pixels.getHeight()-1), Colour{});
}

TEST_F(Matrix2DBasics, IsBlank)
{
    EXPECT_TRUE(pixels.isBlank());
}

TEST_F(Matrix2DBasics, IsNotBlank)
{
    pixels.set(0, 0, Colour{ 1.f, 0.f, 1.f });
    EXPECT_FALSE(pixels.isBlank());
}

TEST_F(Matrix2DBasics, SettingAndGetting)
{
    auto c = Colour{ .5f, .2f, .75f };
    size_t x = pixels.getWidth()-1, y = pixels.getHeight()-1;
    pixels.set(x, y, c);
    EXPECT_EQ(pixels.get(x, y), c);
    auto c2 = Colour{ 0.25f, 0.3f, 0.66f };
    pixels.set(7, 15, c2);
    EXPECT_EQ(pixels.get(7, 15), c2);
}

TEST_F(Matrix2DBasics, GetRawMatrixData)
{
    auto c = Colour{ .5f, .2f, .75f };
    size_t x = pixels.getWidth()-1, y = pixels.getHeight()-1;
    pixels.set(x, y, c);
    auto data = pixels.getMatrix()[x][y];
    EXPECT_EQ(data, c);
    auto c2 = Colour{ 0.25f, 0.3f, 0.66f };
    pixels.set(7, 15, c2);
    EXPECT_EQ(pixels.get(7, 15), c2);
}

TEST_F(Matrix2DBasics, SetAllElementsToSomeValue)
{
    // setting all elements of the matrix to some value
    auto c = Colour{.5f, .2f, .75f};
    pixels.setAllElementsTo(c);
    EXPECT_EQ(pixels.get(0, 0), c);
    EXPECT_EQ(pixels.get(pixels.getWidth()-1, pixels.getHeight()-1), c);
}