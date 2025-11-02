////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Intersections
///     Tracks intersections between rays and shape geometry in a scene
///     Stacy Gaudreau
///     03.07.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <list>


namespace rt
{
class Shape;

////////////////////////////////////////////////////////////////////////////////////////////////////
struct Intersection
{
    /// @brief Contains the time (t) an intersection with a Shape() takes place at.
    Intersection(double t, Shape* shape);
    /// @brief Contains the time (t) an intersection with a Shape() takes place at, storing the
    /// coordinates of intersection on the face, in the case of a triangle.
    Intersection(double t, Shape* shape, double u, double v);
    /// @brief Construct an empty (missed) intersection, which pertains to no shape at all.
    Intersection();
    bool operator<(const Intersection& b) const;
    bool operator==(const Intersection& b) const;
    /// Move assignment
    //Intersection& operator=(Intersection&& b);
    /// Copy assignment
    //Intersection& operator=(const Intersection& b);
    /// @brief Generates a missed hit Intersection type. Used in hit detection.
    static Intersection makeMissedHit();

    double t;
    Shape* shape;
    double u, v;  /// Coordinates an intersection took place at on the Triangle() or SmoothTriangle()

    /// @brief True if this Intersection() is a visible "hit" in the scene.
    [[nodiscard]] inline bool isHit() { return t >= 0.0; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class Intersections
{
  public:
    /// Construct a collection of intersection objects.
    Intersections();
    explicit Intersections(Intersection& i);
    explicit Intersections(const std::vector<Intersection>& ints);
    /// Concatenate two collections of Intersections.
    friend Intersections operator+(const Intersections& A, const Intersections& B);
    /// Add a new intersection to the collection.
    void add(Intersection& intersection);
    /// @brief Get the nth intersection in the collection.
    inline Intersection operator()(size_t n) const { return intersections.at(n); }
    /// @brief Get the nth intersection in the collection.
    inline Intersection& operator()(size_t n) { return intersections[n]; }
    /// @brief Get a const version of the intersections to iterate over.
    inline const std::vector<Intersection>& getIntersections() { return intersections; }
    /// Get the number of intersections.
    [[nodiscard]] size_t count() const;
    /// @brief True if there are no intersections.
    [[nodiscard]] inline bool isEmpty() { return intersections.empty(); };
    /// Find the significant visible intersection, aka: "hit".
    [[nodiscard]] Intersection findHit();
    /// Sort a vector of intersections by ascending t value.
    static void sortIntersectionsAscendingTime(std::vector<Intersection>& intersections);

  private:
    std::vector<Intersection> intersections;
    /// Sorts intersections vector in ascending order of t.
    void sortIntersections();
};
}