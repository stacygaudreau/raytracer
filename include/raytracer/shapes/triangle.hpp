////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Triangle
///     Triangle geometry: basic and smoothed
///     Stacy Gaudreau
///     11.22.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "raytracer/shapes/shape.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Triangle: public Shape
{
  public:
    /// @brief Basic triangle with a flat face. Surface normal is the same at each point on face.
    Triangle(Tuple p1, Tuple p2, Tuple p3);

    Intersections localIntersect(Ray localRay) override;
    Tuple localNormalAt(Tuple localPoint, Intersection iHit = {}) override;

    inline Tuple getNormal() { return normal; }
    inline Tuple getEdge1() { return e1; }
    inline Tuple getEdge2() { return e2; }
    inline Tuple getP1() { return p1; }
    inline Tuple getP2() { return p2; }
    inline Tuple getP3() { return p3; }

  protected:
    Tuple p1, p2, p3;   /// this triangle's three points in the world
    Tuple e1;           /// edge 1
    Tuple e2;           /// edge 2
    Tuple normal;       /// normal vector anywhere on the triangle surface
};


////////////////////////////////////////////////////////////////////////////////////////////////////
class SmoothTriangle: public Triangle
{
  public:
    /// @brief Triangle with a smooth, interpolated surface normal. Uses u, v components.
    SmoothTriangle(Tuple p1, Tuple p2, Tuple p3, Tuple n1, Tuple n2, Tuple n3);

    inline Tuple getN1() { return n1; }
    inline Tuple getN2() { return n2; }
    inline Tuple getN3() { return n3; }

    Tuple localNormalAt(Tuple localPoint, Intersection iHit = {}) override;

  protected:
    Tuple n1, n2, n3;   /// the normal vector for each point
};
}