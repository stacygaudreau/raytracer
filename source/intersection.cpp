#include "intersection.h"
#include "shapes.h"


namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Intersection
////////////////////////////////////////////////////////////////////////////////////////////////////
Intersection::Intersection() : t(0.0), shape(nullptr), u(0.0), v(0.0) {}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersection::Intersection(double t, Shape* shape) : t(t), shape(shape), u(0.0), v(0.0) {}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersection::Intersection(double t, Shape* shape, double u, double v)
: t(t), shape(shape), u(u), v(v)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
bool Intersection::operator<(const Intersection& b) const
{
    return t < b.t;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
bool Intersection::operator==(const Intersection& b) const
{
    return t == b.t && shape == b.shape && u == b.u && v == b.v;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
Intersection Intersection::makeMissedHit()
{
    return { -1.0, nullptr };
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Intersections
////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections::Intersections() : intersections({}) {}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections::Intersections(Intersection& i) : intersections({})
{
    add(i);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections::Intersections(const std::vector<Intersection>& ints) : intersections({})
{
    for (const auto& i : ints) { intersections.push_back(i); }
    sortIntersections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Intersections::add(Intersection& intersection)
{
    intersections.push_back(intersection);
    sortIntersections();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t Intersections::count() const
{
    return intersections.size();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
Intersection Intersections::findHit()
{
    for (auto& i : intersections)
    {
        if (i.isHit()) { return i; }
    }
    // if we've reached here, there is no hit..
    return Intersection::makeMissedHit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Intersections::sortIntersectionsAscendingTime(std::vector<Intersection>& intersections)
{
    // 1. convert to list
    std::list<Intersection> listOfInts{};
    for (const auto& i : intersections) { listOfInts.push_back(i); }
    // 2. sort in place
    listOfInts.sort();
    // 3. write back to vector
    // (this might be faster if iterate and copy to orig. vector?? -> need to profile.)
    intersections.clear();
    for (const auto& i : listOfInts) { intersections.push_back(i); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Intersections::sortIntersections()
{
    sortIntersectionsAscendingTime(intersections);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections operator+(const Intersections& A, const Intersections& B)
{
    Intersections AB{};
    for (const auto& i : A.intersections) { AB.intersections.push_back(i); }
    for (const auto& i : B.intersections) { AB.intersections.push_back(i); }
    AB.sortIntersections();
    return AB;
}

}