#include "gtest/gtest.h"
#include "raytracer/materials.h"
#include "raytracer/matrix.h"
#include "raytracer/world.h"
#include "raytracer/sphere.h"
#include "raytracer/rays.h"
#include "raytracer/plane.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// World Basics
////////////////////////////////////////////////////////////////////////////////////////////////////
class WorldBasics: public ::testing::Test
{
  protected:
    World w{};
    PointLight light{Point{-10.0, 10.0, -10.0}, Colour{1., 1., 1.}};
    Material m1{{0.8, 1.0, 0.6}, 0.1, 0.7, 0.2};
    Sphere s1{}, s2{};

    void SetUp() override {
        s1.setMaterial(m1);
        s2.setTransform(Transform::scale(.5, .5, .5));
        w.addLight(light);
        w.addShape(&s1);
        w.addShape(&s2);
    }
};

TEST_F(WorldBasics, WorldIsConstructed)
{
    World w{};
    EXPECT_TRUE(w.isEmpty());
    EXPECT_FALSE(w.hasLighting());
}

TEST_F(WorldBasics, DefaultWorld)
{
    EXPECT_TRUE(w.hasLighting());
    EXPECT_FALSE(w.isEmpty());
    EXPECT_TRUE(w.containsObject(s1));
    EXPECT_TRUE(w.containsObject(s2));
    EXPECT_DOUBLE_EQ(w.getShape(0)->getMaterial().ambient, 0.1);
}

TEST_F(WorldBasics, DefaultWorldConstructor)
{
//    World def = World::DefaultWorld();
//    auto s1 = dynamic_cast<Sphere*>(def.getShape(0));
//    auto s2 = dynamic_cast<Sphere*>(def.getShape(1));
//    EXPECT_EQ(s1->getMaterial().colour, Colour(0.8, 1.0, 0.6));
//    auto l = def.getLight();
//    EXPECT_EQ(l.position, Point(-10., 10., -10.));
}


// temp commented out
//TEST_F(WorldBasics, RayIntersectsWorld)
//{
//    Ray ray{Point{0, 0, -5}, Vector{0, 0, 1}};
//    Intersections xs = w.intersect(ray);
//    ASSERT_EQ(xs.count(), 4);
//    EXPECT_DOUBLE_EQ(xs(0).t, 4.0);
//    EXPECT_DOUBLE_EQ(xs(1).t, 4.5);
//    EXPECT_DOUBLE_EQ(xs(2).t, 5.5);
//    EXPECT_DOUBLE_EQ(xs(3).t, 6.0);
//}

TEST_F(WorldBasics, PrecomputesStateOfIntersection)
{
    Ray r{Point{0, 0, -5}, Vector{0, 0, 1}};
    Sphere shape{};
    Intersection i{4, &shape};
    Intersections xs{ i };
    IntersectionState state = IntersectionState{i, r, xs};
    EXPECT_EQ(state.t, i.t);
    EXPECT_EQ(state.shape, *i.shape);
    EXPECT_EQ(state.point, Point(0, 0, -1));
    EXPECT_EQ(state.eye, Vector(0, 0, -1));
    EXPECT_EQ(state.normal, Vector(0, 0, -1));
}

TEST_F(WorldBasics, HitWhenIntIsOutsideShape)
{
    Ray r{Point{0, 0, -5}, Vector{0, 0, 1}};
    Sphere shape{};
    Intersection i{4, &shape};
    Intersections xs{ i };
    IntersectionState state = IntersectionState{i, r, xs};
    EXPECT_FALSE(state.isInsideShape);
}

TEST_F(WorldBasics, HitWhenIntIsInsideShape)
{
    Ray r{Point{0, 0, 0}, Vector{0, 0, 1}};
    Sphere shape{};
    Intersection i{1, &shape};
    Intersections xs{ i };
    IntersectionState state = IntersectionState{i, r, xs};
    EXPECT_EQ(state.point, Point(0, 0, 1));
    EXPECT_EQ(state.eye, Vector(0, 0, -1));
    EXPECT_TRUE(state.isInsideShape);
    EXPECT_EQ(state.normal, Vector(0, 0, -1));
}


TEST_F(WorldBasics, ShadingAnIntersectionOutsideShape)
{
    Ray r{Point{0, 0, -5}, Vector{0, 0, 1}};
    Intersection i{4, &s1};
    Intersections xs{ i };
    Colour pixel = w.shadeIntersection(i, r, xs, World::MAX_RAYS);
    EXPECT_EQ(pixel, Colour(.38066, .47583, .2855));
}

TEST_F(WorldBasics, ShadingAnIntersectionInsideShape)
{
    Ray r{Point{0, 0, 0}, Vector{0, 0, 1}};
    Intersection i{0.5, &s2};
    Intersections xs{ i };
    w.setLight(PointLight{ Point{0, .25, 0}, Colour{1, 1, 1} });
    Colour pixel = w.shadeIntersection(i, r, xs, 1);
    EXPECT_EQ(pixel, Colour(.90498, .90498, .90498));
}

TEST_F(WorldBasics, PixelWhenTracedRayMisses)
{
    Ray r{Point{0, 0, -5}, Vector{0, 1, 0}};
    Colour pixel{w.traceRayToPixel(r, World::MAX_RAYS)};
    EXPECT_EQ(pixel, Colour(0, 0, 0));
}

TEST_F(WorldBasics, PixelWhenTracedRayHits)
{
    Ray r{Point{0, 0, -5}, Vector{0, 0, 1}};
    Colour pixel{w.traceRayToPixel(r, World::MAX_RAYS)};
    EXPECT_EQ(pixel, Colour(.38066, .47583, .2855));
}

TEST_F(WorldBasics, PixelWhenHitIsBehindTracedRay)
{
    s1.setAmbient(1.0);
    s2.setAmbient(1.0);
    Ray r{Point{0, 0, .75}, Vector{0, 0, -1}};
    Colour pixel{w.traceRayToPixel(r, World::MAX_RAYS)};
    EXPECT_EQ(pixel, s2.getMaterial().colour);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// World Shadows
////////////////////////////////////////////////////////////////////////////////////////////////////
class WorldShadows: public ::testing::Test
{
  protected:
    World w{};
    PointLight light{Point{-10, 10, -10}, Colour{1, 1, 1}};
    Material m1{{0.8, 1.0, 0.6}, 0.1, 0.7, 0.2};
    Sphere s1{}, s2{};

    void SetUp() override {
        // default world
        s1.setMaterial(m1);
        s2.setTransform(Transform::scale(.5, .5, .5));
        w.addLight(light);
        w.addShape(&s1);
        w.addShape(&s2);
    }
};

TEST_F(WorldShadows, NoShadowWhenNothingColinear)
{
    // a point is considered in shadow when a ray cast from the point to a light source
    // manages to intersect any object
    // here we test that the point is not shadowed when there are no colinear objects btwn
    // point and light source
    auto p = Point{ 0, 10, 0 };
    EXPECT_FALSE(w.isPointInShadow(p));

}

TEST_F(WorldShadows, IsShadowWhenObjectBtwnPointAndLight)
{
    // an object is hit by the shadow ray cast to the light by the point
    auto p = Point{ 10, -10, 10 };
    EXPECT_TRUE(w.isPointInShadow(p));
}

TEST_F(WorldShadows, NoShadowWhenShapeOptsOut)
{
    // an object is hit by the shadow ray cast to the light by the point
    auto p = Point{ 10, -10, 10 };
    s1.setCastsShadow(false);
    EXPECT_FALSE(w.isPointInShadow(p));
}

TEST_F(WorldShadows, NoShadowWhenIntersectsLight)
{
    // ray hits light; not an object in the world
    auto p = Point{ -20, 20, -20 };
    EXPECT_FALSE(w.isPointInShadow(p));
}

TEST_F(WorldShadows, NoShadowWhenObjectBehindPoint)
{
    auto p = Point{ -2, 2, -2 };
    EXPECT_FALSE(w.isPointInShadow(p));
}

TEST_F(WorldShadows, ShadesIntersectionInShadow)
{
    World world{};
    PointLight light{ Point{0, 0, -10}, {1, 1, 1} };
    world.setLight(light);
    Sphere s1{};
    world.addShape(&s1);
    Sphere s2{};
    s2.setTransform(Transform::translation(0.0, 0.0, 10.0));
    world.addShape(&s2);
    Ray r{ Point{0, 0, 5}, Vector{0, 0, 1} };
    Intersection i{ 4, &s2 };
    Intersections xs{ i };
    auto c = world.shadeIntersection(i, r, xs, World::MAX_RAYS);
    EXPECT_EQ(c, (Colour{0.1, 0.1, 0.1}));
}

TEST_F(WorldShadows, IntersectionHitCreatesAddtlOffsetPoint)
{
    // in order to make shadows properly, we need a secondary hit Point, offset
    // a very small amount (EPSILON) above the normal surface of the object
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    Sphere s{};
    s.setTransform(Transform::translation(0., 0., 1.));
    Intersection i{ 5, &s };
    Intersections xs{ i };
    IntersectionState state{ i, r, xs};
    double offsetPointZ = state.pointAboveSurface.z;
    EXPECT_TRUE(offsetPointZ < (-EPSILON/2.0));
    EXPECT_TRUE(state.point.z > offsetPointZ);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// World Reflections
////////////////////////////////////////////////////////////////////////////////////////////////////
class WorldReflections: public WorldBasics
{
};

TEST_F(WorldReflections, BlackIsReturnedFromNonReflectiveMaterial)
{
    Ray r{ Point{0, 0, 0}, Vector{0, 0, 1} };
    s2.setAmbient(1.0);
    Intersection i{1, &s2};
    Intersections xs{ i };
    IntersectionState iState{ i, r, xs };
    Colour c = w.getReflectedColour(iState, World::MAX_RAYS);
    EXPECT_EQ(c, Colour(0, 0, 0));
}

TEST_F(WorldReflections, ColorIsReturnedFromReflectiveMaterial)
{
    Plane plane{};
    plane.setReflectivity(0.5);
    plane.setTransform(Transform::translation(0., -1., 0.));
    w.addShape(&plane);
    Ray r{ Point{0, 0, -3}, Vector{0, -HALF_SQRT_2, HALF_SQRT_2} };
    Intersection i{ SQRT_2, &plane };
    Intersections xs{ i };
    IntersectionState iState{ i, r, xs };
    Colour c = w.getReflectedColour(iState, World::MAX_RAYS);
    EXPECT_EQ(c, Colour(0.190332, 0.237915, 0.142749));
}

TEST_F(WorldReflections, IntersectionIsShadedWithReflectiveMaterial)
{
    Plane plane{};
    plane.setReflectivity(0.5);
    plane.setTransform(Transform::translation(0., -1., 0.));
    w.addShape(&plane);
    Ray r{ Point{0, 0, -3}, Vector{0, -HALF_SQRT_2, HALF_SQRT_2} };
    Intersection i{ SQRT_2, &plane };
    Intersections xs{ i };
    IntersectionState iState{ i, r, xs };
    Colour c = w.shadeIntersectionState(iState, World::MAX_RAYS);
    EXPECT_EQ(c, Colour(0.87677, 0.92436, 0.82918));
}

TEST_F(WorldReflections, RayRecursivelyBouncesBtwnMirrors)
{
    // two fully reflective mirror surfaces are placed facing one-another, and
    //  a ray bounced btwn. them results in what might be infinite recursive calls
    //  of getReflectedColour() and traceRayToPixel() ..
    World w2{};
    w2.addLight(PointLight{ Point{0, 0, 0}, Colour{1, 1, 1} });
    Plane upper{}, lower{};
    lower.setReflectivity(1.0);
    upper.setReflectivity(1.0);
    lower.setTransform(Transform::translation(0., -1., 0.));
    w2.addShape(&lower);
    upper.setTransform(Transform::translation(0., 1., 0.));
    w2.addShape(&upper);
    Ray r{ Point{0, 0, 0}, Vector{0, 1, 0} };
    // we expect a call to traceRayToPixel to not crash
    EXPECT_NO_THROW(w2.traceRayToPixel(r, World::MAX_RAYS));
}

TEST_F(WorldReflections, ReflectedColourAtMaximumRecursiveDepth)
{
    // black should be returned once max recursive depth is reached
    Plane plane{};
    plane.setReflectivity(0.5);
    plane.setTransform(Transform::translation(0., -1., 0.));
    w.addShape(&plane);
    Ray r{Point{0, 0, -3}, Vector{0, -HALF_SQRT_2, HALF_SQRT_2}};
    Intersection i{ SQRT_2, &plane };
    Intersections xs{ i };
    IntersectionState iState{ i, r, xs };
    Colour c = w.getReflectedColour(iState, 0);
    EXPECT_EQ(c, Colour(0, 0, 0));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// World Refractions
////////////////////////////////////////////////////////////////////////////////////////////////////
class WorldRefractions: public WorldBasics
{
  protected:

    class TestPattern: public Pattern
    {
      public:
        TestPattern(Colour a, Colour b): Pattern(a, b) {};

        Colour colourAt(Tuple point) override
        {
            return { point.x, point.y, point.z };
        };
    };

    TestPattern pattern{{}, {}};
};

TEST_F(WorldRefractions, OpaqueSurface)
{
    // a ray refracted by an opaque surface should be black
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    Intersection i1{ 4.0, &s1 }, i2{ 6.0, &s1 };
    Intersections xs{{ i1, i2 }};
    IntersectionState iState{ xs(0), r, xs };
    Colour c = w.getRefractedColour(iState, 5);
    EXPECT_EQ(c, Colour(0, 0, 0));
}

TEST_F(WorldRefractions, BlackReturnedAtMaxRecursiveDepth)
{
    // a ray refracted by an opaque surface should be black
    s1.setRefraction(1.0, 1.5);
    Ray r{ Point{0, 0, -5}, Vector{0, 0, 1} };
    Intersection i1{ 4.0, &s1 }, i2{ 6.0, &s1 };
    Intersections xs{{ i1, i2 }};
    IntersectionState iState{ xs(0), r, xs };
    Colour c = w.getRefractedColour(iState, 0);
    EXPECT_EQ(c, Colour(0, 0, 0));
}

TEST_F(WorldRefractions, TotalInternalReflection)
{
    // this is when light enters a new material at a sufficiently acute angle, with the
    //  new material having a lower refractive index than the parent material. The light then
    //  bounces back in the parent material. eg: light emitting within water and hitting air on
    //  the surface, bouncing back into the water.
    //
    //  we test here that black is returned when total internal reflection occurs.
    s1.setRefraction(1.0, 1.5);
    Ray r{ Point{0, 0, HALF_SQRT_2}, Vector{0, 1, 0} };
    Intersection i1{ -HALF_SQRT_2, &s1 }, i2{ HALF_SQRT_2, &s1 };
    Intersections xs{{ i1, i2 }};
    IntersectionState iState{ xs(1), r, xs }; // we look at 2nd intersection b/c we're inside the sphere
    Colour c = w.getRefractedColour(iState, 5);
    EXPECT_EQ(c, Colour(0, 0, 0));
}

TEST_F(WorldRefractions, ColourReturnedFromRefractedRay)
{
    // the refracted colour in all other cases will spawn a secondary ray
    //   and return its colour
    World world{};
    Material m{};
    m.ambient = 1.0;
    m.setPattern(&pattern);
    Sphere a{}, b{};
    a.setMaterial(m);
    b.setTransform(Transform::scale(.5, .5, .5));
    b.setRefraction(1.0, 1.5);
    world.addLight(PointLight{ Point{-10, 10, -10}, Colour{1, 1, 1} });
    world.addShape(&a);
    world.addShape(&b);
    Ray r{ Point{0, 0, 0.1}, Vector{0, 1, 0} };
    Intersection i1{ -0.9899, &a }, i2{ -0.4899, &b }, i3{ 0.4899, &b }, i4{ 0.9899, &a };
    Intersections xs{{ i1, i2, i3, i4 }};
    IntersectionState iState{ xs(2), r, xs };
    Colour c = world.getRefractedColour(iState, 5);
    EXPECT_EQ(c, Colour(0, 0.99888, 0.04725));
}

TEST_F(WorldRefractions, IntersectionShadedWithTransparentMaterial)
{
    // a ray is refracted off a glass floor, hitting a coloured ball
    Plane floor{};
    floor.setTransform(Transform::translation(0., -1., 0.));
    floor.setRefraction(0.5, 1.5);
    w.addShape(&floor);
    Sphere ball{};
    ball.setColour({1, 0, 0});
    ball.setAmbient(0.5);
    ball.setTransform(Transform::translation(0., -3.5, -0.5));
    w.addShape(&ball);
    Ray r{ Point{0, 0, -3}, Vector{0, -HALF_SQRT_2, HALF_SQRT_2} };
    Intersection i{ SQRT_2, &floor };
    Intersections xs{ i };
    IntersectionState iState{ xs(0), r, xs };
    Colour c = w.shadeIntersectionState(iState, 5);
    EXPECT_EQ(c, Colour(0.93642, 0.68642, 0.68642));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Fresnel Reflectance
////////////////////////////////////////////////////////////////////////////////////////////////////
class FresnelReflectance: public WorldBasics
{
  protected:
};

TEST_F(FresnelReflectance, SchlickApproxUnderTotalInternalReflection)
{
    Sphere s = Sphere::glassySphere();
    Ray r{ Point{ 0, 0, HALF_SQRT_2 }, Vector{ 0, 1, 0 } };
    Intersection i1{ -HALF_SQRT_2, &s }, i2{ HALF_SQRT_2, &s };
    Intersections xs{{ i1, i2 }};
    IntersectionState iState{ xs(1), r, xs };
    const double reflectance = World::getSchlickReflectance(iState);
    EXPECT_EQ(reflectance, 1.0);
}

TEST_F(FresnelReflectance, SchlickWithPerpendicularAngle)
{
    Sphere s = Sphere::glassySphere();
    Ray r{ Point{ 0, 0, 0 }, Vector{ 0, 1, 0 } };
    Intersection i1{ -1, &s }, i2{ 1, &s };
    Intersections xs{{ i1, i2 }};
    IntersectionState iState{ xs(1), r, xs };
    const double reflectance = World::getSchlickReflectance(iState);
    EXPECT_DOUBLE_EQ(reflectance, 0.04);
}

TEST_F(FresnelReflectance, AcuteAngleSchlickAndN2_GT_N1)
{
    Sphere s = Sphere::glassySphere();
    Ray r{ Point{ 0, 0.99, -2 }, Vector{ 0, 0, 1 } };
    Intersection i1{ 1.8589, &s };
    Intersections xs{{ i1 }};
    IntersectionState iState{ xs(0), r, xs };
    const double reflectance = World::getSchlickReflectance(iState);
    EXPECT_TRUE(APPROX_EQ(reflectance, 0.4887308));
}

TEST_F(FresnelReflectance, IntersectionShadedWithReflectiveANDTransparentMaterial)
{
    // a ray is refracted off a transparent AND reflective floor, hitting a coloured ball
    Ray r{ Point{0, 0, -3}, Vector{0, -HALF_SQRT_2, HALF_SQRT_2} };
    Plane floor{};
    floor.setTransform(Transform::translation(0., -1., 0.));
    floor.setRefraction(0.5, 1.5);
    floor.setReflectivity(0.5);
    w.addShape(&floor);
    Sphere ball{};
    ball.setColour({1, 0, 0});
    ball.setAmbient(0.5);
    ball.setTransform(Transform::translation(0., -3.5, -0.5));
    w.addShape(&ball);
    Intersection i{ SQRT_2, &floor };
    Intersections xs{ i };
    IntersectionState iState{ xs(0), r, xs };
    Colour c = w.shadeIntersectionState(iState, 5);
    EXPECT_EQ(c, Colour(0.93391, 0.69643, 0.69243));
}

