#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include "raytracer/utils/utils.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Represents a single RGB colour pixel, with all the basic operations to support mixing and
/// modifying pixels in the world.
////////////////////////////////////////////////////////////////////////////////////////////////////
struct Colour
{
    Colour(double red, double green, double blue);
    Colour() = default;

    /// subtract two colours
    friend Colour operator-(const Colour& a, const Colour& b);
    /// add two colours
    friend Colour operator+(const Colour& a, const Colour& b);
    /// multiply a colour by a scalar
    friend Colour operator*(const Colour& c, const double s);
    /// multiply two colours (Hadamard Product)
    friend Colour operator*(const Colour& a, const Colour& b);
    /// colour equivalence
    friend bool operator==(const Colour& a, const Colour& b);
    /// print a stream representing this colour
    friend std::ostream& operator<<(std::ostream& os, const Colour& c);

    /// @brief print a representation of this colour to stdout
    static void print(const Colour& colour)
    {
        std::cout << "RGB: " << colour.R << "/" << colour.G << "/" << colour.B << "\n";
    }

    /// Colour() to PPM 8bit format pixel data
    static std::string toPPM8b(const Colour& colour);
    /// Convert R, G or B value to a PPM clamped bit integer
    static unsigned int rgbToPPM(const double rgb, const unsigned int maxVal = 255);

    double R, G, B;
};
}



