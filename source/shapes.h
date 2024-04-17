////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Shapes
///     Provides base geometric shape class.
///     Stacy Gaudreau
///     03.07.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "tuples.h"
#include "matrix.h"
#include "materials.h"
#include "intersection.h"
#include "rays.h"
#include "patterns.h"

#include <memory>

namespace rt
{

class Intersections;
class Group;

////////////////////////////////////////////////////////////////////////////////////////////////////
class Shape
{
  public:
    /// Construct a new Shape object at the origin.
    Shape(): Shape(Point{}) {};
    virtual ~Shape() = default;
    /// Construct a new Shape object at the specified position.
    explicit Shape(Tuple position);
    /// @brief Intersect this Shape() with a Ray().
    /// @return Collection of Intersections from the cast Ray.
    inline Intersections intersect(Ray worldRay) {
        // transform the worldRay into a local object-space ray before calling localIntersect
        return localIntersect(worldRay.transform(inverseTransform));
    }
    /// @brief Calculate the normal vector at a specified **world** point on this shape
    /// @param worldPoint A world point on this shape.
    /// @param iHit The "hit" intersection.
    /// @return The normal vector at this point on the shape.
    Tuple normalAt(Tuple worldPoint, Intersection iHit = {});
    /// Compare identity. Are a and b the same shape?
    friend bool operator== (const Shape& a, const Shape& b);
    /// Get the transformation matrix applied to this Shape
    [[nodiscard]] TransformationMatrix getTransform() const;
    /// Set the transformation matrix applied to this Shape
    void setTransform(TransformationMatrix t);
    /// Set the material for this Shape to be rendered with
    virtual void setMaterial(Material newMaterial);
    /// Get the material this Shape is rendered with
    [[nodiscard]] inline Material getMaterial() const { return material; }
    /// @brief Set the colour of this Shape's material.
    inline virtual void setColour(Colour colour) { material.colour = colour; }
    /// @brief Set the ambient lighting amount on this Shape's Material().
    inline void setAmbient(double ambientAmount) { material.ambient = ambientAmount; }
    /// @brief Set the material diffuse property for this Shape.
    inline void setDiffuse(double diffuseAmount) { material.diffuse = diffuseAmount; }
    /// @brief Set the material specular property for this Shape.
    inline void setSpecular(double specularAmount) { material.specular = specularAmount; }
    /// @brief Set the material reflectivity for this Shape.
    inline void setReflectivity(double reflectivityAmount)
                               { material.reflectivity = reflectivityAmount; }
    /// @brief Set the refractive material properties for this Shape.
    inline void setRefraction(double transparency, double refraction) {
        material.transparency = transparency;
        material.refraction = refraction;
    }
    /// @brief True if the shape's material is reflective.
    [[nodiscard]] inline bool isReflective() const { return material.reflectivity > 0.0; };
    [[nodiscard]] inline bool isTransparent() const { return !APPROX_EQ(material.transparency, 0.0); };
    /// @brief Set whether this Shape should cast a shadow in the world or not.
    inline void setCastsShadow(bool newCastsShadow) { castsShadow = newCastsShadow; }
    [[nodiscard]] inline bool getCastsShadow() const { return castsShadow; }
    /// @brief Set the optional pattern the material on this shape should use.
    inline void setPattern(Pattern* pattern) { material.setPattern(pattern); };
    /// Apply lighting to this shape and compute a single pixel from it.
    Colour lightPixel(Light lighting, Tuple pWorld, Tuple vEye, Tuple vNormal, bool isShadowed);
    /// @brief Transform a world point to this Shape's object space.
    inline Tuple transformPoint(Tuple worldPoint) { return inverseTransform * worldPoint; }
    /// @brief Convert a world point to this Shape's object space, recursively traversing through
    /// any parent Group() shapes in the process.
    Tuple worldToObject(Tuple worldPoint);
    /// @brief Convert an object space normal to world space. Takes into account any parent Group()
    /// shapes.
    Tuple normalToWorld(Tuple objectNormal);
    /// @brief Intersect a *locally transformed/object space* ray with this Shape.
    virtual Intersections localIntersect(Ray localRay) = 0;
    /// @brief Calculate the normal vector in *locally transformed/object space*.
    virtual Tuple localNormalAt(Tuple localPoint, Intersection iHit) = 0;

    /// @brief Get the parent group of this Shape.
    inline Group& getGroup() { return *parent; };
    /// @brief Query whether this Shape belongs to a group (ie: has a parent).
    inline bool isGrouped() { return parent != nullptr; }
    /// @brief Set the parent Group() of this Shape.
    inline void setGroup(Group* newGroup) { parent = newGroup; }
    /// @brief Test whether this shape includes another shape.
    virtual bool includes(Shape* s) const { return this == s; }


  protected:
    Tuple position; /// position of the Shape in the Scene right now
    TransformationMatrix transformation; /// the transformation to be applied during raycasting
    TransformationMatrix inverseTransform;  /// cached inverse transform matrix
    Material material;  /// The surface material to render this shape with.
    bool castsShadow; /// flag which lets shapes opt out of casting shadows
    Group* parent{ nullptr };  /// pointer to the parent group (if any) this Shape belongs to
};

}
