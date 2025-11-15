#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>

#include "raytracer/renderer/colour.hpp"
#include "raytracer/math/matrix_2d.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Canvas
{
  public:
    /**
     * @brief Pixel buffer of an image to be rendered. Supports writing to file as PPM.
     * @param width
     * @param height
     */
    Canvas(uint32_t width, uint32_t height);
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const {  return height; }
    bool isBlank();
    void writePixel(uint32_t x, uint32_t y, Colour colour);
    void setAllPixelsTo(Colour colour);
    Colour pixelAt(uint32_t x, uint32_t y);
    /// @brief Generates a PPM-compatible header string for this canvas's pixel matrix.
    std::string generatePPMHeader() const;
    /// @brief Generate a Portable PixMap data string for the entire pixel matrix in this Canvas().
    std::string toPPM() const;
    /// @brief Generates a row of PPM data for a given y from the pixel matrix.
    std::string generatePPMDataRow(uint32_t y) const;
    /// @brief Writes to file a Portable PixMap image
    bool writePPMToFile(const std::string& file) const;

  private:
    uint32_t width, height;
    Matrix2D<Colour> pixels;
};
}