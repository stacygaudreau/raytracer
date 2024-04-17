#include "plane.h"
#include <cmath>

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Plane::localNormalAt(Tuple localPoint, Intersection iHit)
{
    (void)iHit;
    (void)localPoint;
    return Vector{ 0, 1, 0 };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections Plane::localIntersect(Ray localRay)
{
    Intersections intersections{};
    const auto directionY = localRay.getDirection().y;
    // if the ray is parallel to the plane, the y-component of its direction
    // is zero (or extremely close to it, ie: EPSILON)
    if (std::abs(directionY) >= EPSILON)
    {
        // ray is *not* parallel to plane -> one intersection exists
        const double t = -localRay.getOrigin().y / directionY;
        Intersection i{ t, this };
        intersections.add(i);
    }
    return intersections;
}
}
