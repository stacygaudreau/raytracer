#include "raytracer/environment/world.hpp"
#include "raytracer/environment/lighting.hpp"
#include "raytracer/shapes/sphere.hpp"


namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// IntersectionState
////////////////////////////////////////////////////////////////////////////////////////////////////
IntersectionState::IntersectionState(Intersection& i, Ray& ray, Intersections& xs)
:   shape(*i.shape),
    t(i.t),
    point(ray.position(t)),
    eye(-ray.getDirection()),
    normal(shape.normalAt(point, i)),
    isInsideShape(Tuple::dot(normal, eye) < 0)
{
    // invert the normal if we are inside the shape object, so that the shading
    //  will illuminate the surface properly
    if (isInsideShape)
        normal = -normal;
    //
    pointAboveSurface = point + (normal * EPSILON);
    pointBelowSurface = point - (normal * EPSILON);
    // reflecting the ray's direction vector around the shape's normal is how we get
    //  the reflection vector
    vReflect = Vector::reflect(ray.getDirection(), normal);
    findRefractiveIndices(i, xs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void IntersectionState::findRefractiveIndices(Intersection& i, Intersections& xs)
{
    // compute n1 and n2 refractive indices
    for (const Intersection& x: xs.getIntersections()) {
        // find n1
        if (x == i)
        {
            if (refractedShapes.empty())
                n1 = 1.0;
            else
                n1 = refractedShapes.back()->getMaterial().refraction;
        }
        // remove shape if exiting object, else, it is entering, so push it on
        const int indexOfShape = refractedShapesContains(x.shape);
        if (indexOfShape >= 0)
            refractedShapes.erase(refractedShapes.begin() + indexOfShape);
        else
            refractedShapes.push_back(x.shape);
        // find n2
        if (x == i)
        {
            if (refractedShapes.empty())
                n2 = 1.0;
            else
                n2 = refractedShapes.back()->getMaterial().refraction;
            break; // finally, we terminate if it's the hit
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// World
////////////////////////////////////////////////////////////////////////////////////////////////////
World::World() {}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool World::isEmpty() const
{
    return objects.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool World::hasLighting() const
{
    return !lights.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void World::addLight(const Light& light)
{
    lights.push_back(std::make_shared<Light>(light));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void World::addShape(Shape* shape)
{
    objects.push_back(shape);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void World::setLight(const Light& light)
{
    lights.insert(lights.begin(), std::make_shared<Light>(light));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Light World::getLight()
{
    return *lights.front();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool World::containsObject(const Shape& shape)
{
    bool containsObjectOrNot{false};
    for (const auto& o: objects) {
        if (*o == shape) {
            containsObjectOrNot = true;
            break;
        }
    }
    return containsObjectOrNot;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections World::intersect(Ray ray)
{
    // intersect each object in the World with a Ray,
    //  building a collection of aggregated Intersections
    Intersections ints{};
    for(const auto& o: objects) {
        ints = ints + o->intersect(ray);
    }
    return ints;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour World::shadeIntersectionState(IntersectionState iState, size_t nRaysRemain)
{
    const bool isShadowed = isPointInShadow(iState.pointAboveSurface);
    const Colour surface = iState.shape.lightPixel(getLight(), iState.pointAboveSurface,
                                                         iState.eye, iState.normal, isShadowed);
    const Colour reflected = getReflectedColour(iState, nRaysRemain);
    const Colour refracted = getRefractedColour(iState, nRaysRemain);
    if (iState.shape.isReflective() && iState.shape.isTransparent())
    {
        // fresnel effect required; use Schlick approximation
        const double reflectance = getSchlickReflectance(iState);
        return surface + reflected * reflectance
                       + refracted * (1 - reflectance);
    }
    else
        return surface + reflected + refracted;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour World::traceRayToPixel(Ray ray, size_t nRaysRemain)
{
    Intersections xs = intersect(ray);
    Intersection hit = xs.findHit();
    return hit.isHit() ? shadeIntersection(hit, ray, xs, nRaysRemain) : Colour{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool World::isPointInShadow(Tuple point)
{
    const auto vToLight = getLight().position - point;
    double distance = vToLight.magnitude();
    Ray shadowRay{ point, vToLight.normalize() };
    Intersection hit = getHitForRay(shadowRay);
    if (hit.isHit())
    {
        // if tHit < distance, it means the hit lies between the point and the light source,
        //  which therefore means an object was hit (ie: not a light)
        if (hit.t < distance && hit.shape->getCastsShadow())
            return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour World::getReflectedColour(IntersectionState &iState, size_t nRaysRemain)
{
    if (!iState.shape.isReflective() || nRaysRemain <= 0)
        return { 0, 0, 0 };
    else
    {
        // 1. spawn new ray at hit's location, pointing toward vReflect
        const Ray reflectionRay{ iState.pointAboveSurface, iState.vReflect };
        // 2. trace pixel colour of the new ray and multiply it by reflectivity
        const Colour cReflected = traceRayToPixel(reflectionRay, nRaysRemain - 1);
        return cReflected * iState.shape.getMaterial().reflectivity;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour World::getRefractedColour(IntersectionState &iState, size_t nRaysRemain)
{
    if (!iState.shape.isTransparent() || nRaysRemain <= 0)
        return { 0, 0, 0 };
    // find whether there is total internal reflection
    const double nRatio = iState.n1 / iState.n2;
    const double cos_i = Tuple::dot(iState.eye, iState.normal);
    // the sin^2_theta of an incoming ray:
    const double sin2_t = nRatio * nRatio * (1.0 - cos_i * cos_i);
    if (sin2_t > 1.0)
        // there is total internal reflection; return black.
        return { 0, 0, 0 };
    double cos_t = std::sqrt(1.0 - sin2_t);
    Tuple direction = iState.normal * (nRatio * cos_i - cos_t)
                      - (iState.eye * nRatio);
    Ray refractedRay{ iState.pointBelowSurface, direction };
    // the colour of the refracted ray, accounting for any opacity via the
    //  transparency value
    const Colour cRefracted = traceRayToPixel(refractedRay, nRaysRemain - 1)
                              * iState.shape.getMaterial().transparency;
//    std::cout << cRefracted << "\n" << mat << "\n";
    return cRefracted;
}

World World::DefaultWorld()
{
    World w{};
//    Sphere s1{};
//    s1.setMaterial(Material{});
//    w.addShape(&s1);
//    Sphere s2{};
//    s2.setMaterial(Material{});
//    w.addShape(&s2);
//    PointLight light{ Point{-10.0, 10.0, -10.0}, Colour{1., 1., 1.} };
//    w.setLight(&light);
    return w;
}
}
