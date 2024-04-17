#pragma once

#include <string>
#include <iostream>
#include <fstream>

#include "colours.h"
#include "matrix_2d.h"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Canvas
{
  public:
    Canvas(size_t width, size_t height);
    size_t getWidth();
    size_t getHeight();
    bool isBlank();
    void writePixel(size_t x, size_t y, Colour colour);
    void setAllPixelsTo(Colour colour);
    Colour pixelAt(size_t x, size_t y);
    /// @brief Generates a PPM-compatible header string for this canvas's pixel matrix.
    std::string generatePPMHeader() const;
    /// @brief Generate a Portable PixMap data string for the entire pixel matrix in this Canvas().
    std::string toPPM() const;
    /// @brief Generates a row of PPM data for a given y from the pixel matrix.
    std::string generatePPMDataRow(size_t y) const;
    /// @brief Writes to file a Portable PixMap image
    bool writePPMToFile() const;

  private:
    size_t width, height;
    Matrix2D<Colour> pixels;
};
}