#include "gtest/gtest.h"
#include "raytracer/renderer/canvas.hpp"
#include <string>

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Basic Canvas Operations
////////////////////////////////////////////////////////////////////////////////////////////////////

class CanvasBasics : public ::testing::Test
{
  protected:
    Canvas c = Canvas{ 10, 20 };
};

TEST_F(CanvasBasics, CreateCanvas)
{
    EXPECT_EQ(c.getWidth(), 10);
    EXPECT_EQ(c.getHeight(), 20);
}

TEST_F(CanvasBasics, WritePixels)
{
    // test writing pixels to the canvas
    auto red = Colour{ 1.f, 0.f, 0.f };
    c.writePixel(2, 3, red);
    EXPECT_EQ(c.pixelAt(2, 3), red);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Portable PixMap (PPM) Image Formatting Tests
////////////////////////////////////////////////////////////////////////////////////////////////////

class CanvasToPPM : public ::testing::Test
{
  protected:
    Canvas canvas = Canvas(5, 3);
    Colour c1 = Colour{ 1.5, 0., 0. };
    Colour c2 = Colour{ 0., .5, 0. };
    Colour c3 = Colour{ -.5, 0., 1. };

  public:
    void SetUp() override
    {
        canvas.writePixel(0, 0, c1);
        canvas.writePixel(2, 1, c2);
        canvas.writePixel(4, 2, c3);
    }
};

TEST_F(CanvasToPPM, GeneratesPPMHeader)
{
    // canvas can construct a PPM format header
    auto header = canvas.generatePPMHeader();
    EXPECT_EQ(header, "P3\n5 3\n255\n");
}

TEST_F(CanvasToPPM, GeneratesPPMRowData)
{
    auto row = canvas.generatePPMDataRow(0);
    const char* expected =
        "255 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n";
    EXPECT_EQ(row, expected);
}

TEST_F(CanvasToPPM, IsNotLongerThan70Chars)
{
    // canvas to PPM splits long lines (>70 chars) into multiple rows
    Canvas canvas2 = Canvas(10, 2);
    Colour c2 = Colour{1., .25, .6};
    canvas2.setAllPixelsTo(c2);
    const auto ppm = canvas2.toPPM();
    const char* expected =
        "255 64 153 255 64 153 255 64 153 255 64 153 255 64 153 255 64 153 255\n"
        "64 153 255 64 153 255 64 153 255 64 153\n"
        "255 64 153 255 64 153 255 64 153 255 64 153 255 64 153 255 64 153 255\n"
        "64 153 255 64 153 255 64 153 255 64 153\n";
    EXPECT_EQ(ppm, expected);
}

TEST_F(CanvasToPPM, PPMFileIsWritten)
{
    // writing to PPM file is a success
    auto c = Canvas(5, 3);
    c.writePPMToFile();
}
