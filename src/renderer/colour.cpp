#include "raytracer/renderer/colour.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
Colour::Colour(double red, double green, double blue) : R(red), G(green), B(blue) {}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour operator-(const Colour& a, const Colour& b)
{
    return { a.R - b.R, a.G - b.G, a.B - b.B };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour operator+(const Colour& a, const Colour& b)
{
    return { a.R + b.R, a.G + b.G, a.B + b.B };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour operator*(const Colour& c, const double s)
{
    return { c.R * s, c.G * s, c.B * s };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour operator*(const Colour& a, const Colour& b)
{
    return { a.R * b.R, a.G * b.G, a.B * b.B };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const Colour& a, const Colour& b)
{
    return APPROX_EQ(a.R, b.R) && APPROX_EQ(a.G, b.G) && APPROX_EQ(a.B, b.B);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const Colour& c)
{
    os << c.R << "r " << c.G << "g " << c.B << "b";
    return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int Colour::rgbToPPM(const double rgb, const unsigned int maxVal)
{
    return static_cast<unsigned int>(std::clamp(
        static_cast<int>(std::round(rgb * static_cast<int>(maxVal))), 0, static_cast<int>(maxVal)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::string Colour::toPPM8b(const Colour& colour)
{
    const auto R = std::clamp(static_cast<int>(std::round(colour.R * 255.)), 0, 255);
    const auto G = std::clamp(static_cast<int>(std::round(colour.G * 255.)), 0, 255);
    const auto B = std::clamp(static_cast<int>(std::round(colour.B * 255.)), 0, 255);
    return std::to_string(R) + " " + std::to_string(G) + " " + std::to_string(B);
}
}