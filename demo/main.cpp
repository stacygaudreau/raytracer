#include <iostream>

#include "raytracer/shapes/sphere.hpp"
#include "raytracer/shapes/plane.hpp"
#include "raytracer/environment/world.hpp"
#include "raytracer/environment/camera.hpp"
#include "raytracer/renderer/renderer.hpp"
#include "raytracer/materials/textures.hpp"
#include "raytracer/materials/material.hpp"

using namespace rt;


void run_generative_texture_demo()
{
    World world{};
    Colour white{0.968, 0.968, 0.968}, black{0.110, 0.119, 0.124};
    Colour copper{0.722, 0.451, 0.20};
//    Colour turquoise{0.0, 0.80784, 0.81961};

    Texture::Waves waves{};
    waves.setAmplitude(0.15);
    waves.setFrequency(4);
    Material m2{ copper };
    m2.setTexture(&waves);
    m2.specular = 0.6;
    m2.transparency = 0.0;
    m2.refraction = 1.1;
    m2.reflectivity = 0.2;
    m2.diffuse = 0.6;
    m2.ambient = 0.3;
    Sphere s2{};
    s2.setTransform(Transform::translation(-2.75, 3.5, 1.0)
                            * Transform::rotateZ(-QUARTER_PI)
                            * Transform::scale(0.75, 0.75, 0.75));
    s2.setMaterial(m2);
    world.addShape(&s2);


    Plane floor{};
    CheckersPattern tiles{ black, white };
    Material mf{{0.0, 0.08, 0.1}};
    mf.reflectivity = 0.25;
    mf.ambient = 0.7;
    mf.specular = 0.7;
//    mf.setPattern(&tiles);
    Texture::Noise tf{};
    tf.setDensity(5);
    tf.setAmplitude(0.08);
    mf.setTexture(&tf);
    floor.setMaterial(mf);
    floor.setTransform(Transform::rotateX(-QUARTER_PI));
    world.addShape(&floor);

    Texture::Noise noise{};
    noise.setAmplitude(2.0);
    noise.setDensity(5.0);
    noise.setOctaves(3);
    noise.setNoiseType(Texture::Noise::NoiseType::cellular);
    noise.setTransform(
            Transform::scale(0.25, 0.25, 0.25)
                    * Transform::rotateZ(QUARTER_PI));
    noise.setFractalType(Texture::Noise::FractalType::ridged);
    noise.setWarpAmplitude(200.0);
    noise.setWarpType(Texture::Noise::WarpType::simplex2);
    noise.setWarpDensity(0.05);

    Material m{ { 0.78, 0.05, 0.1 } };
    m.specular = 0.7;
    m.shininess = 30;
    m.transparency = 0.0;
    m.refraction = 1.25;
    m.reflectivity = 0.0;
    m.diffuse = 0.4;
    m.ambient = 0.4;
    m.setTexture(&noise);
    Sphere s{};
    s.setMaterial(m);
    s.setTransform(Transform::translation(0.0, 1.25, -2.0)
                   * Transform::scale(1.25, 1.25, 1.25));


    world.addShape(&s);

    // light source
    auto light = PointLight{ Point{ -5., 3., -7 }, Colour{1.0, 0.97, 0.92 } };
    world.setLight(light);
    // camera
    auto camera = Camera{ 1080, 1080, THIRD_PI };
    camera.setTransform(Transform::viewTransform({0.0, 1.1, -7.2},
                                                 {0.0, 0.75, 0.0},
                                                 {0.0, 1.0, 0.0}));
    // render the result to a canvas
    Renderer raytracer{ 16 };
    auto image = raytracer.render(camera, world);
    image.writePPMToFile();
    std::cout << "Canvas saved to PPM file.\n";
}

int main()
{
    run_generative_texture_demo();
    return 1;
}