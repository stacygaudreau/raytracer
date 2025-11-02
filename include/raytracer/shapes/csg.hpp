////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Constructive Solid Geometry
///     Set theory operations for combining Shapes with union, intersect and difference.
///     Stacy Gaudreau
///     10.12.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include "raytracer/shapes/shape.hpp"
#include "raytracer/shapes/group.hpp"

namespace rt
{

////////////////////////////////////////////////////////////////////////////////////////////////////
class CSG:  public Group
{
  public:
    enum Operation {
        UNION,
        INTERSECT,
        DIFFERENCE
    };

    CSG(Shape* a, Shape* b, Operation operation = Operation::UNION);

    /// @brief Construct a union operation, ie: a + b, between two shapes.
    static CSG Union(Shape* a, Shape* b);
    /// @brief Construct an intersection operation between two shapes.
    static CSG Intersect(Shape* a, Shape* b);
    /// @brief Construct a difference operation, ie: a - b, between two shapes.
    static CSG Difference(Shape* a, Shape* b);

    inline Operation getOperation() { return op; }
    inline Shape& getLeft() { return *children.at(0); }
    inline Shape& getRight() { return *children.at(1); }
    /// @brief Get whether a Ray() intersection is permitted, given where the hit occurs.
    /// @param leftWasHit True if the left shape was hit, false if right was hit.
    /// @param hitInsideLeft True when the Ray hits *inside* the left shape.
    /// @param hitInsideRight True when the Ray hits *inside* the right shape.
    bool intersectionAllowed (bool leftWasHit, bool hitInsideLeft, bool hitInsideRight);
    /// @brief Filter a list of Intersections(), producing a subset which are allowed by the
    /// CSG geometry contained in this shape.
    Intersections filterIntersections(Intersections& xs);

    /// @brief Test whether this CSG includes another given Shape.
    bool includes(Shape* s) const override;
    /// @brief Intersect a *locally transformed/object space* ray with this Shape.
    Intersections localIntersect(Ray localRay) override;

  private:
    Operation op;
};

}  // namespace rt