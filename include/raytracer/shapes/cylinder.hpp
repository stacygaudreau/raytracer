////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Cylinder
///     Geometry for cylinders
///     Stacy Gaudreau
///     11.21.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "raytracer/shapes/shape.hpp"
#include "raytracer/utils/utils.hpp"

namespace rt
{
class Cylinder : public Shape
{
  public:
    Cylinder() : Shape() {}

    Intersections localIntersect(Ray localRay) override;
    /// @brief Calculate the normal vector in *locally transformed/object space*.
    Tuple localNormalAt(Tuple localPoint, Intersection iHit = {}) override;


    [[nodiscard]] inline bool getIsClosed() const { return isClosed; }
    inline void setIsClosed(bool newIsClosed) { isClosed = newIsClosed; }
    /// @brief Set the height of the cylinder, specifying top and bottom values on the y-axis to
    /// truncate at.
    inline void setHeight(double topY, double bottomY)
    {
        maxY = topY;
        minY = bottomY;
    }
    /// @brief Set the total height of the cylinder, truncating the top and bottom.
    inline void setHeight(double height) { setHeight(height / 2., -height / 2.); }
    /// @brief Return the minY truncation point.
    [[nodiscard]] inline double getMinY() const { return minY; }
    [[nodiscard]] inline double getMaxY() const { return maxY; }


  private:
    bool isClosed{ false };  // true when the cylinder should have closed end caps rendered
    double minY{ -INF };     // minimum bound to truncate cylinder with
    double maxY{ INF };      // maximum bound to truncate cylinder with
    /// @brief Checks to see if intersection at time t is within the radius of the
    /// cylinder from the y-axis.
    inline static bool checkCap(Ray& r, double t);
    /// @brief Intersect a given Ray with the caps of this cylinder.
    inline void intersectCaps(Ray& r, Intersections& xs);
};
}
