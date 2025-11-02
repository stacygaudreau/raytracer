#pragma once

#include "raytracer/utils.h"
#include <ostream>

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
struct Tuple
{
    Tuple(double x, double y, double z, double w) : x(x), y(y), z(z), w(w){};
    Tuple() = default;

    friend bool operator==(const Tuple& a, const Tuple& b);
    friend Tuple operator+(const Tuple& a, const Tuple& b);
    friend Tuple operator*(const Tuple& a, const double& s);
    friend Tuple operator/(const Tuple& a, const double& s);
    friend Tuple operator-(const Tuple& a, const Tuple& b);

    /// negate the tuple (ie: subtract it from the zero vector)
    Tuple operator-() const;
    double magnitude() const;
    /// Get the normalized version of this Tuple.
    Tuple normalize() const;
    static double dot(const Tuple& a, const Tuple& b);
    bool isPoint() const;
    bool isVector() const;
    /// Mimics array indexing. Hacky. Should refactor tuple internals eventually.
    double& operator()(size_t i);
    /// Mimics array indexing. Hacky. Should refactor tuple internals eventually.
    double operator()(size_t i) const;
    /// Prints a stream representing this Tuple
    friend std::ostream& operator<<(std::ostream& os, const Tuple& tuple);

    double x, y, z, w;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
struct Vector : public Tuple
{
    Vector(double x, double y, double z) : Tuple(x, y, z, 0.0) {};
    Vector() : Tuple(0.0, 0.0, 0.0, 0.0) {};
    /// Reflect this vector about a given normal vector.
    Tuple reflect(Tuple normal);
    /// Compute a reflection vector, given a source vector (v) and normal vector (n).
    static Tuple reflect(const Tuple v, const Tuple n);
};


////////////////////////////////////////////////////////////////////////////////////////////////////
struct Point : public Tuple
{
    Point(double x, double y, double z) : Tuple(x, y, z, 1.0) {};
    Point() : Tuple(0.0, 0.0, 0.0, 1.0) {};
};


////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple cross(const Tuple& a, const Tuple& b);
}