#include "lighting.h"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Light
////////////////////////////////////////////////////////////////////////////////////////////////////
Light::Light(Tuple position, Colour colour)
:   position(position),
    colour(colour)
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const Light& a, const Light& b)
{
    return &a == &b;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// PointLight
////////////////////////////////////////////////////////////////////////////////////////////////////
PointLight::PointLight(Tuple position, Colour intensity)
:   Light(position, intensity)
{}
}
