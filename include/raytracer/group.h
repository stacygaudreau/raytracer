////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Group
///     Grouping shapes
///     Stacy Gaudreau
///     11.21.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "raytracer/shapes.h"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Group : public Shape
{
  public:
    Group() : Shape() {}

    /// @brief Intersect a *locally transformed/object space* ray with this Group.
    Intersections localIntersect(Ray localRay) override;
    /// @brief Calculate the normal vector in *locally transformed/object space*.
    Tuple localNormalAt(Tuple localPoint, Intersection iHit) override;
    /// @brief Get whether the group is empty of other shapes or not.
    inline bool isEmpty() { return children.empty(); }
    /// @brief Add a child Shape to this grouping.
    inline void addChild(Shape* shape)
    {
        children.push_back(shape);
        shape->setGroup(this);
    }
    /// @brief Get the nth child in the grouping.
    inline Shape& getChild(size_t n) { return *children.at(n); }
    /// @brief Set the material for all of the children in this group at once.
    void setMaterial(Material newMaterial) override;
    /// @brief Test whether this Group includes another given Shape.
    bool includes(Shape* s) const override;

  protected:
    std::vector<Shape*> children;
};
}