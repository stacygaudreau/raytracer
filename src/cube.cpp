#include "raytracer/cube.h"

namespace rt
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Cube
////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Cube::localNormalAt(Tuple localPoint, Intersection iHit)
{
    (void)iHit;
    // the cube face the normal is on is the component of the point with
    //  the largest absolute value, ie: closest to 1.0 or -1.0
    const double absX   = std::abs(localPoint.x);
    const double absY   = std::abs(localPoint.y);
    const double absZ   = std::abs(localPoint.z);
    const double maxVal = std::max({ absX, absY, absZ });
    Tuple v{};
    if (absX == maxVal)
        v = Vector{ localPoint.x, 0, 0 };
    else if (absY == maxVal)
        v = Vector{ 0, localPoint.y, 0 };
    else
        v = Vector{ 0, 0, localPoint.z };
    return v;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections Cube::localIntersect(Ray localRay)
{
    auto x = checkAxis(localRay.getOrigin().x, localRay.getDirection().x);
    auto y = checkAxis(localRay.getOrigin().y, localRay.getDirection().y);
    auto z = checkAxis(localRay.getOrigin().z, localRay.getDirection().z);

    const double tMin = std::max({ x.min, y.min, z.min });
    const double tMax = std::min({ x.max, y.max, z.max });

    Intersections xs{};
    // the ray hits the square only if tMin < tMax
    if (tMin < tMax)
    {
        Intersection min{ tMin, this }, max{ tMax, this };
        xs.add(min);
        xs.add(max);
    }
    // we return empty if the ray was outside the square (since tMax < tMin is a contradiction)
    return xs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Cube::IntersectionTimes Cube::checkAxis(double origin, double direction)
{
    IntersectionTimes t{};
    const double tMin_num = (-1.0 - origin);
    const double tMax_num = (1.0 - origin);

    t.min = tMin_num / direction;
    t.max = tMax_num / direction;

    if (t.min > t.max)
    {
        const double temp = t.min;
        t.min             = t.max;
        t.max             = temp;
    }

    return t;
}

}