////////////////////////////////////////////////////////////////////////////////////////////////////
///     @name Raytracer Libs: Renderer
///     @brief Multithreaded image renderer
///     @author Stacy Gaudreau
///     @date 25.11.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include "raytracer/math/matrix.hpp"
#include "raytracer/renderer/ray.hpp"
#include "raytracer/environment/world.hpp"
#include "canvas.hpp"
#include "raytracer/environment/camera.hpp"

#include <cstdint>
#include <chrono>
#include <thread>
#include <iostream>
#include <syncstream>
#include <cmath>

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class RenderThread
{
  public:
    /// @brief A single rendering thread. Handles ray tracing a batch of Ray() cast by a Camera()
    /// view into a given Canvas() image.
    /// @param threadId The index for this worker thread in the pool.
    /// @param jobSize Total number of pixels the worker will render.
    /// @param nThreads Total number of threads. This is the value the worker will use to increment
    /// its cursor through the pixel matrix being processed.
    /// @param camera The camera view to render.
    /// @param world The world the camera is inside.
    /// @param canvas The image canvas to render pixels to.
    RenderThread(size_t threadId, size_t jobSize, size_t nThreads,
                 Camera& camera, World& world, Canvas& image)
    :   nPxDone(0),
        nThread(threadId),
        pxInc(nThreads),
        nPxTotal(jobSize),
        camera(camera),
        hSize(camera.getHSize()),
        vSize(camera.getVSize()),
        world(world),
        image(image)
    {
        // report progress made every 1%
        pxReportInterval = (size_t)std::ceil((double)nPxTotal / 100.0);
        if (pxReportInterval < 5) pxReportInterval = 5;
    }

    /// @brief Execute the job for this render thread.
    void start()
    {
        thread = std::thread{ &RenderThread::render, this };
    }

    /// @brief Render pixels to the image canvas.
    void render()
    {
        size_t nPx{};
        while (nPx < nPxTotal)
        {
            auto x = getPixelX(getPixelIndex(nPx));
            auto y = getPixelY(getPixelIndex(nPx));
            auto px = world.traceRayToPixel(camera.getRayForCanvasPixel(x, y),
                                            World::MAX_RAYS);
            image.writePixel(x, y, px);

            if (nPx % pxReportInterval == 0)
                reportNPixelsDone(nPx);

            nPx++;
//            tout << "<" << nThread << "> i: " << nPx << ", (" << x << "," << y << "): " << px << "\n";
//            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        // job is done; so make sure it is reported
        reportNPixelsDone(nPxTotal);
    }

    /// @brief Report a new number of pixels done for reading by parent render object.
    inline void reportNPixelsDone(size_t nPixelsDone)
    {
        infoMutex.lock();
            nPxDone = nPixelsDone;
        infoMutex.unlock();
    }

    inline void printThreadInfo()
    {
        std::cout << "<RenderThread:"<< nThread << "> spawned. Job size: " << nPxTotal
                  << " pixels.\n";
    }

    /// @brief Convert job pixel index to overall image pixel index.
    inline size_t getPixelIndex(size_t i) { return nThread + i * pxInc; }
    /// @brief Get x of pixel coordinate in the overall matrix, for a given index.
    inline size_t getPixelY(size_t i) {
        return (size_t)std::floor((double)i / (double)hSize); }
    /// @brief Get y of pixel coordinate in the overall matrix, for a given index.
    inline size_t getPixelX(size_t i) {
        return i - hSize * (size_t)std::floor((double)i / (double)hSize);
    }

    std::thread thread;
    std::mutex infoMutex;   // protects render info members
    size_t nPxDone;

  private:
    size_t nThread; // index of this thread in the total rendering job
    size_t pxInc;    // amount to increment this thread's pixel index in the matrix each time
    size_t nPxTotal;
    Camera& camera;
    size_t hSize, vSize;
    World& world;
    Canvas& image;

    size_t pxReportInterval;    // the pixel count to report new numbers at
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class Renderer
{
  public:
    /// @brief Create a multi-threaded image rendering instance, able to render Camera() views into
    /// image files.
    Renderer(size_t _nThreads): nThreads(_nThreads)
    {
    }

    /// @brief Set the image size to be rendered.
    inline void setImageSize(size_t h, size_t v) {
        hSize = h;
        vSize = v;
        nPxTotal = hSize * vSize;
    }
    /// @brief Set the number of threads to use when rendering.
    inline void setNThreads(size_t newNThreads) { nThreads = newNThreads; }

    /// @brief Start a rendering job for a Camera() view, returning a Canvas() image when complete.
    /// @param camera The camera view to render.
    /// @param world The world to render the camera view in.
    Canvas& render(Camera& camera, World& world)
    {
        // blank image set to the camera view size
        image = std::make_unique<Canvas>(camera.getHSize(), camera.getVSize());
        setImageSize(image->getWidth(), image->getHeight());

        // spawn new worker threads
        workers.clear();
        for (size_t n{}; n < nThreads; ++n)
            workers.push_back(std::make_unique<RenderThread>(n, getJobSize(n), nThreads,
                                                             camera, world, *image));

        std::cout << "<Renderer> Begin render of " << hSize << "x" << vSize << " image.\n";
        std::cout << "<Renderer> Spawning " << nThreads << " threads to render "
                  << nPxTotal << "px total...\n";

        // start rendering
        auto t0 = std::chrono::high_resolution_clock::now();
        nPxDone = 0;
        percentDone = 0.0;

        // spawn render info thread
        std::thread infoThread{ &Renderer::displayRenderInfo, this };
        // start render workers
        for (auto& worker: workers)
        {
            worker->printThreadInfo();
            worker->start();
        }

        // await render completion
        for (auto& worker: workers)
            worker->thread.join();
        // await render completion
        infoThread.join();

        // render complete
        auto t1 = std::chrono::high_resolution_clock::now();
        auto tSeconds = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0);
        auto tMinutes = std::chrono::duration_cast<std::chrono::minutes>(t1 - t0);
        std::cout << "\n///////////////////////////////////////////////////////////////\n";
        std::cout << "// Image render complete! \n";
        std::cout << "// Rendering took " << tSeconds.count() << " seconds, or "
                  << tMinutes.count() << " mins total.\n";
        std::cout << "///////////////////////////////////////////////////////////////\n";

        return *image;
    }

    /// @brief Get the job size in pixels for a given thread number.
    inline size_t getJobSize(size_t nThread)
    {
        size_t nPx{};
        for (size_t i = nThread; i < nPxTotal; i += nThreads)
            nPx++;
        return nPx;
    }

    /// @brief A thread that displays real time information about an ongoing render.
    void displayRenderInfo()
    {
        std::osyncstream tout(std::cout); // thread safe std::cout
        using namespace std::chrono_literals;
        size_t nComplete{};
        std::cout << "\n";
        while (nComplete < nPxTotal)
        {
            // find total pixel progress made by all workers
            nComplete = 0;
            for (auto& worker: workers)
            {
                worker->infoMutex.lock();
                    nComplete += worker->nPxDone;
                worker->infoMutex.unlock();
            }
            auto percent = 100.0 * (double)nComplete / (double)nPxTotal;
            percentDone = percent;
            std::cout << "\r<Rendering> " << percent << "%";
            std::this_thread::sleep_for(250ms);
        }
    }
    /// @brief Get the percent complete of the current render.
    inline double getPercentDone() const { return percentDone; }

  private:
    std::vector<std::unique_ptr<RenderThread>> workers;

    size_t nThreads{ 1 };
    size_t hSize{}, vSize{};
    size_t nPxTotal{};
    double percentDone{ 0.0 };
    size_t nPxDone{};

    std::unique_ptr<Canvas> image;  /// the rendered image
};
}
