#include "shapes.h"
#include "group.h"


namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Shape
////////////////////////////////////////////////////////////////////////////////////////////////////
Shape::Shape(Tuple position)
:   position(position),
    transformation(TransformationMatrix::identity()),
    inverseTransform(transformation.inverse()),
    material(Material{}),
    castsShadow(true)
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const Shape& a, const Shape& b)
{
    return &a == &b;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
TransformationMatrix Shape::getTransform() const
{
    return transformation;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Shape::setTransform(TransformationMatrix t)
{
    transformation = t;
    inverseTransform = t.inverse();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Shape::setMaterial(Material newMaterial) {
    material = newMaterial;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour Shape::lightPixel(Light lighting, Tuple pWorld, Tuple vEye, Tuple vNormal, bool isShadowed)
{
    return material.lightPixel(lighting, pWorld, worldToObject(pWorld),
                               vEye, vNormal, isShadowed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Shape::normalAt(Tuple worldPoint, Intersection iHit)
{
    const auto localPoint = worldToObject(worldPoint);
    const auto localNormal = localNormalAt(localPoint, iHit);
    Tuple normal{};
    if (material.hasTexture())
        normal = material.getTexture()->applyToNormal(normalToWorld(localNormal), localPoint);
    else
        normal = normalToWorld(localNormal);
    return normal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Shape::worldToObject(Tuple worldPoint)
{
    // TO-DO: refactor to not use recursion
    if (isGrouped())
        worldPoint = parent->worldToObject(worldPoint);
    return transformPoint(worldPoint);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Shape::normalToWorld(Tuple objectNormal)
{
    // TO-DO: refactor to not use recursion
    objectNormal = inverseTransform.transposed() * objectNormal;
    objectNormal.w = 0.0; // hacky way of avoiding having another submatrix operation
    objectNormal = objectNormal.normalize();

    if (isGrouped())
        objectNormal = parent->normalToWorld(objectNormal);

    return objectNormal; // NB: this is actually now the WORLD normal!
}
}