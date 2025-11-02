////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Materials
///     Surface materials for objects
///     Stacy Gaudreau
///     08.07.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "raytracer/math/tuples.hpp"
#include "raytracer/environment/lighting.hpp"
#include "raytracer/renderer/colour.hpp"
#include "patterns.hpp"
#include "raytracer/materials/textures.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Material
////////////////////////////////////////////////////////////////////////////////////////////////////
class Material
{
  public:
    explicit Material(Colour colour={1.0, 1.0, 1.0},
             double ambient=0.1, double diffuse=0.9, double specular=0.9,
             double shininess=200.0, double reflectivity=0.0, double transparency=0.0,
             double refraction=1.0);
    /// Compare equality.
    friend bool operator== (const Material& a, const Material& b);
    /// @brief Apply lighting to this material and compute a single pixel from it.
    Colour lightPixel(Light lighting, Tuple pWorld, Tuple pShape,
                      Tuple vEye, Tuple vNormal, bool isShadowed=false);

    inline void setPattern(Pattern* newPattern) { pattern = newPattern; }
    [[nodiscard]] inline bool hasPattern() const { return pattern != nullptr; }
    inline Pattern* getPattern() { return pattern; }

    inline void setTexture(Texture::Generative* newTexture) { texture = newTexture; }
    [[nodiscard]] inline bool hasTexture() const { return texture != nullptr; }
    inline Texture::Generative* getTexture() { return texture; }



    Colour colour;
    double ambient, diffuse, specular, shininess;
    double reflectivity;
    double transparency, refraction;

  private:
    Pattern* pattern{ nullptr };   /// an optional surface pattern which can be applied
    Texture::Generative* texture{ nullptr };   /// optional generative surface texture
};
}