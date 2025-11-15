#include "raytracer/environment/camera.hpp"
#include <chrono>

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Camera
////////////////////////////////////////////////////////////////////////////////////////////////////
Camera::Camera(uint32_t _hSize, uint32_t _vSize, double fieldOfView)
: _hSize(_hSize),
  _vSize(_vSize),
  hSizeF(static_cast<double>(_hSize)),
  vSizeF(static_cast<double>(_vSize)),
  fieldOfView(fieldOfView)
{
    // the canvas is placed one world unit away from the "front" of the camera
    // we calculate the width of half of the canvas by projecting a triangular FOV
    // out from the camera's "eye"
    halfView    = tan(fieldOfView / 2.);
    aspectRatio = hSizeF / vSizeF;
    // determine width and height of canvas depending on aspect ratio
    if (getAspectIsHorizontal())
    {
        // horiz >= vert
        halfWidth  = halfView;
        halfHeight = halfView / aspectRatio;
    }
    else
    {
        // vert > horiz
        halfWidth  = halfView * aspectRatio;
        halfHeight = halfView;
    }
    // determine pixel size for the canvas (we assume pixels are square)
    pixelSize = (halfWidth * 2.0) / hSizeF;
    // default transform is identity
    setTransform(TransformationMatrix::identity());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Ray Camera::getRayForCanvasPixel(uint32_t pixelX, uint32_t pixelY)
{
    // offset from edge of canvas to pixel centre
    const double xOff = (static_cast<double>(pixelX) + 0.5) * pixelSize;
    const double yOff = (static_cast<double>(pixelY) + 0.5) * pixelSize;
    // coordinates of pixel in worldspace *without* transformation
    // +X is to the left since our camera looks toward -Z
    const double worldX = halfWidth - xOff;
    const double worldY = halfHeight - yOff;
    // xform canvas point and origin, then get the ray's direction vector
    const auto pixel     = inverseTransform * Point{ worldX, worldY, -1.0 };
    const auto origin    = inverseTransform * Point{ 0., 0., 0. };
    const auto direction = (pixel - origin).normalize();

    return { origin, direction };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Camera::setTransform(TransformationMatrix newTransform)
{
    transform = newTransform;
    inverseTransform = transform.inverse();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Canvas Camera::render(World world)
{
    auto t0 = std::chrono::high_resolution_clock::now();
    std::cout << "\nRaytracing of " << _hSize << "x" << _vSize << "px image started. "
              << _hSize * _vSize << " pixels to render...\n";
    uint32_t nRowsDone{};
    Canvas image{ _hSize, _vSize };
    for (uint32_t y{}; y < _vSize; ++y)
    {
        for (uint32_t x{}; x < _hSize; ++x)
        {
            auto ray   = getRayForCanvasPixel(x, y);
            auto pixel = world.traceRayToPixel(ray, World::MAX_RAYS);
            image.writePixel(x, y, pixel);
        }
        nRowsDone++;
        std::cout << nRowsDone << "/" << _vSize << " rows\n";
    }
    auto t1= std::chrono::high_resolution_clock::now();
    auto tSeconds = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0);
    auto tMinutes = std::chrono::duration_cast<std::chrono::minutes>(t1 - t0);
    std::cout << "///////////////////////////////////////////////////////////////\n";
    std::cout << "// Image render complete! \n";
    std::cout << "// Rendering took " << tSeconds.count() << " seconds, or " << tMinutes.count()
              << " mins total.\n";
    std::cout << "///////////////////////////////////////////////////////////////\n";
    return image;
}
}