////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Surface Textures
///     Generative surface textures made by perturbing the surface normal of shapes
///     Stacy Gaudreau
///     11.12.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "raytracer/third_party/noise/FastNoiseLite.h"
#include "raytracer/tuples.h"
#include "raytracer/matrix.h"
#include "raytracer/utils.h"

namespace rt::Texture
{

////////////////////////////////////////////////////////////////////////////////////////////////////
class Generative
{
  public:
    /// @brief A generative surface texture which can be applied to a Material()
    explicit Generative();
    virtual ~Generative() = default;

    /// @brief Applies the surface texture to a given normal, using normal perturbation.
    Tuple applyToNormal(Tuple normal, Tuple point);

    /// @brief Compute the perturbation Vector() at a given point in object space.
    /// @return A modulated normal vector which includes the texture applied to it.
    virtual Tuple getPerturbation(Tuple& point) = 0;

    /// @brief Set a transformation to be applied to the texture itself.
    inline void setTransform(TransformationMatrix newTransform)
    {
        transform = newTransform;
        inverseTransform = newTransform.inverse();
    }

    /// @brief Set the coefficients for the algorithm used to generate the texture.
    inline void setCoefficients(double x, double y, double z) {
        C.x = x;
        C.y = y;
        C.z = z;
    }
    /// @brief Set the coefficient used in generating the texture.
    inline void setCoefficients(double xyz) { C.x = C.y = C.z = xyz; }

    /// @brief Set amplitudes used to apply texture to the material.
    inline void setAmplitude(double x, double y, double z) {
        A.x = x;
        A.y = y;
        A.z = z;
    }
    /// @brief Set amplitude used to apply texture to the material.
    inline void setAmplitude(double xyz) { A.x = A.y = A.z = xyz; }

    struct {
        double x{ 0.5 }, y{ 0.5 }, z{ 0.5 };
    } C;  /// coefficients used in texture algorithm

    struct {
        double x{ 0.25 }, y{ 0.25 }, z{ 0.25 };
    } A;  /// amplitude of texture applied to material

  private:
    TransformationMatrix transform;
    TransformationMatrix inverseTransform;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
class Noise : public Generative
{
  public:
    /// @brief A base generative noisy surface texture.
    Noise();

    enum class FractalType
    {
        none,
        fbm,
        pingpong,
        ridged
    };

    enum class NoiseType
    {
        simplex,
        cellular,
        perlin,
        value,
        cubic
    };

    enum class WarpType
    {
        none,
        simplex,
        simplex2,
        grid
    };

    enum class FractalWarpType
    {
        none,
        progressive,
        independent
    };

    /// @brief Maps fractal type enum to Fast Noise Lite enum type.
    static inline FastNoiseLite::FractalType fractalTypeToFNL(FractalType ftype)
    {
        using FNL = FastNoiseLite::FractalType;
        switch (ftype)
        {
        case FractalType::fbm:
            return FNL::FractalType_FBm;
        case FractalType::pingpong:
            return FNL::FractalType_PingPong;
        case FractalType::ridged:
            return FNL::FractalType_Ridged;
        default:
            return FNL::FractalType_None;
        }
    }

    /// @brief Maps noise type enum to Fast Noise Lite enum type.
    static inline FastNoiseLite::NoiseType noiseTypeToFNL(NoiseType ntype)
    {
        using FNL = FastNoiseLite::NoiseType;
        switch (ntype)
        {
        case NoiseType::simplex:
            return FNL::NoiseType_OpenSimplex2;
        case NoiseType::cellular:
            return FNL::NoiseType_Cellular;
        case NoiseType::perlin:
            return FNL::NoiseType_Perlin;
        case NoiseType::value:
            return FNL::NoiseType_Value;
        case NoiseType::cubic:
            return FNL::NoiseType_ValueCubic;
        default:
            return FNL::NoiseType_OpenSimplex2;
        }
    }

    /// @brief Maps warp type enum to Fast Noise Lite enum type.
    static inline FastNoiseLite::DomainWarpType warpTypeToFNL(WarpType type)
    {
        using FNL = FastNoiseLite::DomainWarpType;
        switch (type)
        {
            case WarpType::simplex: return FNL::DomainWarpType_OpenSimplex2;
            case WarpType::simplex2: return FNL::DomainWarpType_OpenSimplex2Reduced;
            case WarpType::grid: return FNL::DomainWarpType_BasicGrid;
            default: return FNL::DomainWarpType_OpenSimplex2;
        }
    }

    /// @brief Set the density (or frequency) of the noise algorithm.
    inline void setDensity(double newDensity)
    {
        density = newDensity;
        noise.SetFrequency(static_cast<float>(density));
    }
    /// @brief Set the number of octaves of noise used in fractal generation.
    inline void setOctaves(int newNOctaves)
    {
        nOctaves = newNOctaves;
        noise.SetFractalOctaves(newNOctaves);
    }
    /// @brief Set the type of fractal noise to generate.
    inline void setFractalType(FractalType type)
    {
        fractalType = type;
        noise.SetFractalType(fractalTypeToFNL(fractalType));
    }
    /// @brief Set the base type of noise to be generated.
    inline void setNoiseType(NoiseType type)
    {
        noiseType = type;
        noise.SetNoiseType(noiseTypeToFNL(noiseType));
    }

    /// @brief Set the amount of domain warping applied. 0 results in no warping.
    inline void setWarpAmplitude(double amp) {
        warpAmp = amp;
        warpNoise.SetDomainWarpAmp(static_cast<float>(warpAmp));
        warpIsActive = !APPROX_ZERO(warpAmp);
    }

    /// @brief Set the density of domain warping applied.
    inline void setWarpDensity(double newDensity) {
        warpDensity = newDensity;
        warpNoise.SetFrequency(static_cast<float>(warpDensity));
    }

    /// @brief Set the type of domain warping applied.
    inline void setWarpType(WarpType type) {
        warpType = type;
        if (warpType == WarpType::none) {
            setWarpAmplitude(0.0);
            warpIsActive = false;
        }
        else
            warpIsActive = true;
        warpNoise.SetDomainWarpType(warpTypeToFNL(warpType));
    }






    /// @brief Modulate a given normal vector with the presently configured noise function.
    Tuple getPerturbation(Tuple& point) override;

  protected:
    FastNoiseLite noise;    /// primary noise generator
    FastNoiseLite warpNoise;     /// warped noise

  private:
    double density{ .005 };
    int nOctaves{ 1 };
    FractalType fractalType{ FractalType::none };
    NoiseType noiseType{ NoiseType::simplex };
    double warpAmp{ 0.0 };
    bool warpIsActive{ false };
    WarpType warpType{ WarpType::none };  // type of domain warping applied
    double warpDensity{ .01 };
};


////////////////////////////////////////////////////////////////////////////////////////////////////
class Waves : public Generative
{
  public:
    Waves();

    Tuple getPerturbation(Tuple& point) override;

    /// @brief Set the period of one wave, in world distance units.
    //    inline void setPeriod(double period)
    //    {
    //        f = TWO_PI / (period*TWO_PI);
    //    }
    //
    /// @brief Set the frequency of the waves, per world distance unit.
    inline void setFrequency(double f) { frequency = f; }

  private:
    double frequency{ 1.0 };  /// frequency the waves will repeat, per one world distance unit
};

} // NAMESPACE rt::Texture

