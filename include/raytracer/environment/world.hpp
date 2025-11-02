////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: World
///     Contains arbitrary scene geometry and raycasting methods
///     Stacy Gaudreau
///     09.07.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <cstdint>
#include <memory>

#include "raytracer/shapes/shape.hpp"
#include "raytracer/environment/lighting.hpp"
#include "raytracer/renderer/ray.hpp"
#include "raytracer/renderer/intersection.hpp"


namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// IntersectionState
////////////////////////////////////////////////////////////////////////////////////////////////////
struct IntersectionState
{
    /// @brief Encapsulate some pre-computed state about an intersection, for use in shading pixels.
    IntersectionState(Intersection& i, Ray& ray, Intersections& xs);
    Shape& shape;
    double t;
    Tuple point;
    Tuple eye;
    Tuple normal;
    bool isInsideShape;
    Tuple pointAboveSurface;    /// an offset version of main point, slightly above the surface.
    Tuple pointBelowSurface;    /// the point where refracted rays will originate, slightly below the surface
    Tuple vReflect{}; /// reflection vector
    double n1{}, n2{};  /// refraction indices of materials on either side of the intersection

  private:
    std::vector<Shape*> refractedShapes;

    inline int refractedShapesContains(Shape* _shape)
    {
        int indexOfShape{ -1 };
        for (int i{}; i < static_cast<int>(refractedShapes.size()); ++i) {
            if (refractedShapes[i] == _shape)
            {
                indexOfShape = i;
                break;
            }
        }
        return indexOfShape;
    }

    inline void findRefractiveIndices(Intersection& i, Intersections& xs);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// World
////////////////////////////////////////////////////////////////////////////////////////////////////
class World
{
  public:
    World();
    /// @brief Construct the Default World, with a couple of concentric sphere objects and a light
    /// in it.
    static World DefaultWorld();
    /// True if there are no objects in the World.
    [[nodiscard]] bool isEmpty() const;
    /// False if no lights have been added to the World, yet.
    [[nodiscard]] bool hasLighting() const;
    /// Add a Light() to the World.
    void addLight(const Light& light);
    /// Add a Shape() to the World.
    void addShape(Shape* shape);
    /// @brief Get the Shape (if any) at the specified index in the World.
    inline Shape* getShape(size_t index) { return objects.at(index); }
    /// Set the first Light() in the World.
    void setLight(const Light& light);
    /// Get the first Light() for this world.
    Light getLight();
    /// @brief Check if a specific object exists in this World or not.
    bool containsObject(const Shape& shape);
    /// @brief Get an Intersection for a given Ray(), which may or may not be a visible hit on an
    /// object's surface in the World.
    inline Intersection getHitForRay(Ray ray) { return intersect(ray).findHit(); }
    /// @brief Intersect this World() with a Ray() and return the sorted Intersections()
    Intersections intersect(Ray ray);
    /// @brief Compute shading at a given Intersection() with a Ray().
    inline Colour shadeIntersection(Intersection i, Ray ray, Intersections& xs, size_t nRaysRemain) {
        return shadeIntersectionState(IntersectionState{i, ray, xs}, nRaysRemain);
    }
    /// @brief Cast a Ray() into the world and compute a given pixel Colour() for it.
    /// @details This is called color_at() in the book.
    Colour traceRayToPixel(Ray ray, size_t nRaysRemain);
    /// @brief Get whether a given Point() is in the shadow of any objects in the current World.
    bool isPointInShadow(Tuple point);
    /// @brief Get a reflected Colour pixel in the World.
    Colour getReflectedColour(IntersectionState &iState, size_t nRaysRemain);
    /// @brief Get a refracted Colour pixel in the World.
    Colour getRefractedColour(IntersectionState &iState, size_t nRaysRemain);
    /// @brief Shade a precomputed IntersectionState.
    Colour shadeIntersectionState(IntersectionState iState, size_t nRaysRemain);
    /// @brief Get the Schlick approximation of reflectance for the given intersection state.
    inline static double getSchlickReflectance(IntersectionState& i)
    {
        double cos = Tuple::dot(i.eye, i.normal);
        // total internal reflection occurs only if n1 > n2
        if (i.n1 > i.n2)
        {
            const double n = i.n1 / i.n2;
            const double sin2_t = n*n * (1.0 - cos*cos);
            if (sin2_t > 1.0)
                return 1.0;
            const double cos_t = std::sqrt(1.0 - sin2_t);
            // when n1 > n2 we use cos_t instead of cos
            cos = cos_t;
        }
        const double r = ((i.n1 - i.n2) / (i.n1 + i.n2));
        const double r0 = r*r;
        const double cos_1 = (1 - cos);
        return r0 + (1 - r0) * cos_1 * cos_1 * cos_1 * cos_1 * cos_1;
    }

    static constexpr size_t MAX_RAYS{ 4 }; // max number of recursive rays to cast

  private:
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<Shape*> objects;
};
}
