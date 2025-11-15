////////////////////////////////////////////////////////////////////////////////////////////////////
///     @name Raytracer Libs: Camera
///     @brief World cameras to move around and view rendered scenes.
///     @author Stacy Gaudreau
///     @date 13.11.2022
////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <cstdint>

#include "raytracer/math/matrix.hpp"
#include "raytracer/renderer/ray.hpp"
#include "raytracer/environment/world.hpp"
#include "raytracer/renderer/canvas.hpp"


namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class Camera
{
  public:
    /// @brief Create a new camera to place in the world. This is required to view a scene.
    /// @param hSize Horizontal size (in px) of the canvas the scene will be rendered to.
    /// @param vSize Vertical size (in px) of the canvas the scene will be rendered to.
    /// @param fieldOfView An angle which describes the field-of-view of the camera, in radians.
    Camera(uint32_t hSize, uint32_t vSize, double fieldOfView);

    /// @brief Renders a given World into a Canvas image, according to this Camera's view.
    /// @param world The World() to render this camera view in.
    Canvas render(World world);
    /// @brief Generate a new Ray() which casts from this camera, passing through the specified
    /// pixel x, y on the camera's canvas.
    /// @param pixelX x coordinate on the camera canvas the ray will pass through.
    /// @param pixelY y coordinate on the camera canvas the ray will pass through.
    Ray getRayForCanvasPixel(uint32_t pixelX, uint32_t pixelY);
    /// @brief Set the transform matrix for the camera's position and orientation in worldspace.
    void setTransform(TransformationMatrix newTransform);

    void setVSize(uint32_t vSize) noexcept { _vSize = vSize; }
    [[nodiscard]] inline uint32_t getVSize() const noexcept { return _vSize; }
    void setHSize(uint32_t hSize) noexcept { _hSize = hSize; }
    [[nodiscard]] inline uint32_t getHSize() const noexcept { return _hSize; }
    [[nodiscard]] inline double getPixelSize() const noexcept { return pixelSize; }
    /// @brief Returns true when the camera has a horizontal aspect ratio. False if it is vertical.
    [[nodiscard]] inline bool getAspectIsHorizontal() const { return aspectRatio >= 1.0; }
    [[nodiscard]] inline double getFOV() const { return fieldOfView; }
    [[nodiscard]] inline TransformationMatrix getTransform() const { return transform; }

  private:
    uint32_t _hSize, _vSize;
    double hSizeF, vSizeF;  // cached double versions of hSize/vSize
    double fieldOfView;
    TransformationMatrix transform;
    TransformationMatrix inverseTransform;  /// we cache the inverse to save repeated computations
    //
    double halfView;
    double aspectRatio;
    double halfWidth, halfHeight;  // width and height of the canvas, as determined by aspect ratio
    double pixelSize;
};
}