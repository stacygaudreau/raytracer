////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Cubes
///     Axis-aligned Bounding Boxes (AABB) cube geometry.
///     Stacy Gaudreau
///     19.11.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shapes.h"
#include <tuple>

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Cube : public Shape
{
  public:
    Cube() : Shape() {}

    Tuple localNormalAt(Tuple localPoint, Intersection iHit = {}) override;
    Intersections localIntersect(Ray localRay) override;

    struct IntersectionTimes
    {
        double min{}, max{};
    };

    /// @brief Get minimum and maximum intersection times with one of the axis' plane of the cube.
    static IntersectionTimes checkAxis(double origin, double direction);
};
}