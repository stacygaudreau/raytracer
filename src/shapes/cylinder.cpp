#include "raytracer/shapes/cylinder.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Cylinder
////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections Cylinder::localIntersect(Ray localRay)
{
    Intersections xs{};
    const auto dir    = localRay.getDirection();
    const auto origin = localRay.getOrigin();
    const double a    = dir.x * dir.x + dir.z * dir.z;
    if (APPROX_EQ(a, 0.0))
        // ray parallel to y-axis, only possible cap intersection
        intersectCaps(localRay, xs);
    else
    {
        // possible sidewall and/or cap intersections
        const double b            = 2. * origin.x * dir.x + 2. * origin.z * dir.z;
        const double c            = origin.x * origin.x + origin.z * origin.z - 1.;
        const double discriminant = b * b - 4.0 * a * c;
        if (discriminant >= 0.0)
        {
            // find t values for the two intersections
            const double SQRT_D = std::sqrt(discriminant);
            const double TWO_A  = 2.0 * a;
            double t0           = (-b - SQRT_D) / TWO_A;
            double t1           = (-b + SQRT_D) / TWO_A;
            if (t0 > t1) Utils::swap(t0, t1);
            // find y coord at each point of intersection; if it's btwn min and max
            //  bounds, then the ix. is valid
            Intersection i0{ t0, this };
            Intersection i1{ t1, this };
            const double y0 = origin.y + t0 * dir.y;
            if (minY < y0 && y0 < maxY) xs.add(i0);
            const double y1 = origin.y + t1 * dir.y;
            if (minY < y1 && y1 < maxY) xs.add(i1);
        }
        intersectCaps(localRay, xs);
    }
    return xs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Cylinder::localNormalAt(Tuple localPoint, Intersection iHit)
{
    (void)iHit;
    std::unique_ptr<Tuple> normal;
    // end caps are planes, so just like planes they have the same normal anywhere on
    //  their surface
    // find which cap the point belongs to (if any), or whether its on the cylinder walls
    const double distFromY     = localPoint.x * localPoint.x + localPoint.z * localPoint.z;
    const bool withinCapRadius = distFromY < 1.0;
    if (withinCapRadius && localPoint.y >= maxY - EPSILON)
        // top cap
        normal = std::make_unique<Tuple>(Vector{ 0, 1, 0 });
    else if (withinCapRadius && localPoint.y <= minY + EPSILON)
        // bottom cap
        normal = std::make_unique<Tuple>(Vector{ 0, -1, 0 });
    else
        // cylinder walls
        normal = std::make_unique<Tuple>(Vector{ localPoint.x, 0, localPoint.z });
    return *normal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Cylinder::intersectCaps(Ray& r, Intersections& xs)
{
    const auto origin = r.getOrigin();
    const auto dir    = r.getDirection();
    // caps only matter if cylinder is closed
    if (!isClosed || APPROX_EQ(dir.y, 0.0)) return;
    // check for lower cap by intersecting with plane at y=cyl.minY
    double t = (minY - origin.y) / dir.y;
    if (checkCap(r, t))
    {
        Intersection ix{ t, this };
        xs.add(ix);
    }
    // check for upper cap by intersecting with plane at y=cyl.maxY
    t = (maxY - origin.y) / dir.y;
    if (checkCap(r, t))
    {
        Intersection ix{ t, this };
        xs.add(ix);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool Cylinder::checkCap(Ray& r, double t)
{
    const auto origin = r.getOrigin();
    const auto dir    = r.getDirection();
    const double x    = origin.x + t * dir.x;
    const double z    = origin.z + t * dir.z;
    return (x * x + z * z) <= 1.0;
}
}
