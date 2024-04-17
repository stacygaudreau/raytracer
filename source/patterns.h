////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Patterns
///     Creates patterns which can be rendered onto objects
///     Stacy Gaudreau
///     15.11.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "colours.h"
#include "tuples.h"
#include "matrix.h"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Pattern
{
  public:
    Pattern(Colour a, Colour b);
    /// @brief Get the pattern's colour at a given point in pattern space.
    virtual Colour colourAt(Tuple point) = 0;
    /// @brief Get the pattern's colour at a given point in a shape's object space.
    Colour colourAtShape(Tuple pShape);
    /// @brief Set the transformation applied to the pattern.
    inline void setTransform(TransformationMatrix newTransform)
    {
        transform = newTransform;
        inverseTransform = newTransform.inverse();
    }
    /// @brief Get the transformation matrix applied to this pattern.
    inline const TransformationMatrix& getTransform() { return transform; };
    /// @brief Get the inverse of the transformation matrix applied to this pattern.
    inline const TransformationMatrix& getInverseTransform() { return inverseTransform; };

    Colour a, b;

  protected:
    TransformationMatrix transform;
    TransformationMatrix inverseTransform;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
class StripedPattern: public Pattern
{
  public:
    StripedPattern(Colour a, Colour b) : Pattern(a, b) {};
    Colour colourAt(Tuple point) override;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
class GradientPattern: public Pattern
{
  public:
    GradientPattern(Colour a, Colour b): Pattern(a, b) {};
    Colour colourAt(Tuple point) override;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
class RingPattern: public Pattern
{
  public:
    RingPattern(Colour a, Colour b): Pattern(a, b) {};
    Colour colourAt(Tuple point) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class CheckersPattern: public Pattern
{
  public:
    CheckersPattern(Colour a, Colour b): Pattern(a, b) {};
    Colour colourAt(Tuple point) override;
};
}