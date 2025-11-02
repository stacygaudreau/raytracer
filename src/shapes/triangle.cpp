#include "raytracer/shapes/triangle.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Triangle
////////////////////////////////////////////////////////////////////////////////////////////////////
Triangle::Triangle(Tuple p1, Tuple p2, Tuple p3)
:   Shape(),
    p1(p1),
    p2(p2),
    p3(p3),
    e1(p2 - p1),
    e2(p3 - p1),
    normal(cross(e2, e1).normalize())
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections Triangle::localIntersect(Ray localRay)
{
    // ray-triangle intersection algorithm based on
    // https://www.tandfonline.com/doi/abs/10.1080/10867651.1997.10487468
    Intersections xs{};
    const auto dirCrossE2 = cross(localRay.getDirection(), e2);
    const double determinant = Tuple::dot(e1, dirCrossE2);

    if (std::abs(determinant) >= EPSILON)
    {
        const double f = 1.0 / determinant;
        auto p1ToOrigin = localRay.getOrigin() - p1;
        const double u = f * Tuple::dot(p1ToOrigin, dirCrossE2);

        if (u >= 0. && u <= 1.)
        {
            auto origCrossE1 = cross(p1ToOrigin, e1);
            const double v = f * Tuple::dot(localRay.getDirection(), origCrossE1);
            if (v >= 0 && (u + v) <= 1.)
            {
                // we have an intersection. Since it's a triangle, we store the
                // u and v intersection location, for possible
                // interpolation with later
                Intersection x0{
                    f * Tuple::dot(e2, origCrossE1),
                    this, u, v
                };
                xs.add(x0);
            }
        }
    }
    // if we've reached this point, there is no intersection
    return xs;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Triangle::localNormalAt(Tuple localPoint, Intersection iHit)
{
    (void)localPoint;
    (void)iHit;
    return normal;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// SmoothTriangle
////////////////////////////////////////////////////////////////////////////////////////////////////
SmoothTriangle::SmoothTriangle(Tuple p1, Tuple p2, Tuple p3, Tuple n1, Tuple n2, Tuple n3)
:   Triangle(p1, p2, p3),
    n1(n1),
    n2(n2),
    n3(n3)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple SmoothTriangle::localNormalAt(Tuple localPoint, Intersection iHit)
{
    (void)localPoint;
    // interpolate the normal by combining n1...n3 according to u and v components
    return n2 * iHit.u + n3 * iHit.v + n1 * (1 - iHit.u - iHit.v);
}
}
