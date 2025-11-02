#include "raytracer/shapes/csg.hpp"

namespace rt
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSG
////////////////////////////////////////////////////////////////////////////////////////////////////
CSG::CSG(Shape* a, Shape* b, Operation operation)
:   op(operation)
{
    addChild(a);
    addChild(b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSG CSG::Union(Shape* a, Shape* b)
{
    return { a, b, Operation::UNION };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSG CSG::Intersect(Shape* a, Shape* b)
{
    return { a, b, Operation::INTERSECT };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSG CSG::Difference(Shape* a, Shape* b)
{
    return { a, b, Operation::DIFFERENCE };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSG::intersectionAllowed(bool leftWasHit, bool hitInsideLeft, bool hitInsideRight)
{
    bool isAllowed{ false };
    switch (op)
    {
    case Operation::UNION:
        isAllowed = (leftWasHit && !hitInsideRight) || (!leftWasHit && !hitInsideLeft);
        break;
    case Operation::INTERSECT:
        isAllowed = (leftWasHit && hitInsideRight) || (!leftWasHit && hitInsideLeft);
        break;
    case Operation::DIFFERENCE:
        isAllowed = (leftWasHit && !hitInsideRight) || (!leftWasHit && hitInsideLeft);
        break;
    default:
        break;
    }
    return isAllowed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections CSG::filterIntersections(Intersections& xs)
{
    // begin outside both shapes
    bool inLeft{ false };
    bool inRight{ false };

    Intersections fx{ };

    for(auto x: xs.getIntersections())
    {
        // left shape was hit if the hit's shape is part of the left CSG child shape
        bool leftWasHit = children.at(0)->includes(x.shape);
        if (intersectionAllowed(leftWasHit, inLeft, inRight))
            fx.add(x);

        // depending which shape was hit, toggle inLeft or inRight
        if (leftWasHit)
            inLeft = !inLeft;
        else
            inRight = !inRight;
    }

    return fx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSG::includes(Shape* s) const
{
    return children.at(0)->includes(s) || children.at(1)->includes(s);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Intersections CSG::localIntersect(Ray localRay)
{
    const auto xsLeft = children.at(0)->intersect(localRay);
    const auto xsRight = children.at(1)->intersect(localRay);
    auto xs = xsLeft + xsRight;
    return filterIntersections(xs);
}

}  // namespace rt