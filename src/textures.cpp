#include "raytracer/textures.h"
#include <cmath>

namespace rt::Texture
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Generative Texture
////////////////////////////////////////////////////////////////////////////////////////////////////
Generative::Generative()
{
    setTransform(TransformationMatrix::identity());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Generative::applyToNormal(Tuple normal, Tuple point)
{
    // the "up" vector isn't very useful as a cross product if it is pointing in the same direction
    // as the z-axis, so we will consider "up" to be the x-axis if it happens that the normal is
    // roughly the same as the z-axis.
//    static const auto NEG_Z = Vector{ 0., 0., -1. };
//    static const auto POS_X = Vector{ 1., 0., 0. };
//    const auto up = normal == NEG_Z ? POS_X : NEG_Z;
//    // find the u,v directions in the tangent plane
//    const auto V = cross(up, normal).normalize();
//    const auto U = cross(normal, V).normalize();
//    return (normal + (U * P.x) + (V * P.y) + (normal * P.z)).normalize();

    // compute perturbation vector and modulate the normal with it
    auto pTexture = inverseTransform * point;
    const auto P = getPerturbation(pTexture);
    return Vector{ normal.x + P.x, normal.y + P.y, normal.z + P.z }.normalize();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Waves
////////////////////////////////////////////////////////////////////////////////////////////////////
Waves::Waves()
{
    A.x = 0.0;
    A.y = 0.3;
    A.z = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Waves::getPerturbation(Tuple& point)
{
    const double x = fmod((point.y * frequency), 1.0) * TWO_PI;
    const double sin_x = sin(x);
    return Vector{ sin_x * A.x, sin_x * A.y, sin_x * A.z };
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Noise
////////////////////////////////////////////////////////////////////////////////////////////////////
Noise::Noise()
{
    setCoefficients(1.0);
    setDensity(density);
    setOctaves(nOctaves);
    setFractalType(fractalType);
    setNoiseType(noiseType);
    setWarpAmplitude(warpAmp);
    setWarpDensity(warpDensity);
    setWarpType(warpType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Tuple Noise::getPerturbation(Tuple& point)
{
    auto x = static_cast<float>(point.x * C.x);
    auto y = static_cast<float>(point.y * C.y);
    auto z = static_cast<float>(point.z * C.z);
    if (warpIsActive)
        warpNoise.DomainWarp(x, y, z);
    float nx = noise.GetNoise(x, y, z);
    return Vector{ nx * A.x, nx * A.y, nx * A.z };
}


}  // namespace rt