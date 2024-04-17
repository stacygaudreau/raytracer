////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Rays, Raycasting and Intersections
///     Provides Rays, raycasting and intersection geometry
///     Stacy Gaudreau
///     03.07.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <cmath>

#include "tuples.h"
#include "matrix.h"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Ray
{
  public:
    Ray(Tuple origin, Tuple direction);
    [[nodiscard]] Tuple getOrigin() const;
    [[nodiscard]] Tuple getDirection() const;
    /// Get the position at the given distance t along the ray
    Tuple position(double t);
    /// Apply a Transform() Matrix(), returning a new Ray.
    [[nodiscard]] Ray transform(TransformationMatrix t) const;

  private:
    Tuple origin;
    Tuple direction;
};
}


