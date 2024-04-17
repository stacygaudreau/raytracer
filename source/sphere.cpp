#include "sphere.h"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sphere
////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Sphere::localNormalAt(Tuple localPoint, Intersection iHit)
{
    (void) iHit;
    return localPoint - Point{};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections Sphere::localIntersect(Ray localRay)
{
    Intersections intersections;
    // we use the sphere-transformed ray's direction and
    //  origin in our calculations
    const Tuple rayDirection{ localRay.getDirection() };
    const Tuple rayOrigin{ localRay.getOrigin() };
    const auto vSphereToRay = rayOrigin - position;
    const auto a = Tuple::dot(rayDirection, rayDirection);
    const auto b = 2.0 * Tuple::dot(rayDirection, vSphereToRay);
    const auto c = Tuple::dot(vSphereToRay, vSphereToRay) - 1.0;
    const auto discriminant = b * b - 4.0 * a * c;
    const auto SQRT_D = std::sqrt(discriminant);
    const auto TWO_A = a * 2.0;
    if (discriminant >= 0)
    {
        Intersection i1{ (-b - SQRT_D) / TWO_A, this };
        Intersection i2{ (-b + SQRT_D) / TWO_A, this };
        intersections.add( i1 );
        intersections.add( i2 );
    }
    return intersections;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
Sphere Sphere::glassySphere()
{
    Sphere sphere{};
    sphere.setRefraction(1.0, 1.5);
    return sphere;
}
}
