#include "raytracer/materials.h"

#include <cmath>

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Material
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
Material::Material(Colour colour, double ambient, double diffuse, double specular,
                   double shininess, double reflectivity, double transparency, double refraction)
:   colour(colour),
    ambient(ambient),
    diffuse(diffuse),
    specular(specular),
    shininess(shininess),
    reflectivity(reflectivity),
    transparency(transparency),
    refraction(refraction)
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator==(const Material& a, const Material& b)
{
    return    a.colour == b.colour
           && a.ambient == b.ambient
           && a.diffuse == b.diffuse
           && a.specular == b.specular
           && a.shininess == b.shininess
           && a.pattern == b.pattern
           && a.reflectivity == b.reflectivity
           && a.transparency == b.transparency
           && a.refraction == b.refraction;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Colour Material::lightPixel(Light lighting, Tuple pWorld, Tuple pShape,
                            Tuple vEye, Tuple vNormal, bool isShadowed)
{
    Colour colourToUse = hasPattern() ? pattern->colourAtShape(pShape) : colour;
    // add together the material's ambient, diffuse and specular components.
    // components are weighted by the angles between the different vectors.
    // combine surface colour w. the light's colour
    const Colour effectiveColour = colourToUse * lighting.colour;
    // direction to the light source
    Tuple vLight = (lighting.position - pWorld).normalize();
    // the ambient contribution
    Colour ambientColour = effectiveColour * ambient;
    // compute diffuse and specular lighting components...
    Colour diffuseColour{}, specularColour{};
    // cosine of the angle btwn the light and normal vectors
    // a negative # means the light is on the other side of the surface.
    const double lightDotNormal = Tuple::dot(vLight, vNormal);
    if (lightDotNormal >= 0) {
        // light is on this side of the surface, so compute the diffuse
        diffuseColour = effectiveColour * diffuse * lightDotNormal;
        // a negative number means the light reflects away from the eye
        Tuple vReflect = Vector::reflect(-vLight, vNormal);
        const double reflectDotEye = Tuple::dot(vReflect, vEye);
        if (reflectDotEye >= 0) {
            // reflection is visible to the eye, so compute the specular component
            const double reflectAmt = pow(reflectDotEye, shininess);
            specularColour = lighting.colour * specular * reflectAmt;
        }
    }
    // add the three components together for our final shaded pixel result
    return isShadowed ? ambientColour : (ambientColour + diffuseColour + specularColour);
}
}
