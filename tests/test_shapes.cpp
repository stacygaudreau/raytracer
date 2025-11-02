#include "raytracer/shapes.h"
#include "raytracer/sphere.h"
#include "raytracer/plane.h"
#include "gtest/gtest.h"
#include "raytracer/group.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Basic Shape Properties
////////////////////////////////////////////////////////////////////////////////////////////////////
class ShapeBasics: public ::testing::Test
{
  protected:

    class DerivedShape: public Shape
    {
      public:
        DerivedShape(): Shape(){};

        Tuple localNormalAt(Tuple localPoint, Intersection iHit) override
        {
            (void)iHit;
            return Vector{ localPoint.x, localPoint.y, localPoint.z };
        };

        Intersections localIntersect(Ray localRay) override
        {
            objectRay = localRay;
            return Intersections{};
        }

        Ray objectRay{Tuple{}, Tuple{}};

    };

    DerivedShape s{};
};

TEST_F(ShapeBasics, SetShapeDiffuse)
{
    // can set the material diffuse of a shape
    const double diff = 0.56789;
    s.setDiffuse(diff);
    EXPECT_EQ(s.getMaterial().diffuse, diff);
}

TEST_F(ShapeBasics, SetShapeAmbient)
{
    // can set the material ambient amount of a shape
    const double amb = 0.23211231;
    s.setAmbient(amb);
    EXPECT_EQ(s.getMaterial().ambient, amb);
}

TEST_F(ShapeBasics, SetShapeSpecular)
{
    // can set the material specular amount of a shape
    const double spec = 0.928311;
    s.setSpecular(spec);
    EXPECT_EQ(s.getMaterial().specular, spec);
}

TEST_F(ShapeBasics, SetShapeReflectivity)
{
    // can set the material specular amount of a shape
    Plane plane{};
    plane.setReflectivity(0.935);
    EXPECT_EQ(plane.getMaterial().reflectivity, 0.935);
    EXPECT_TRUE(plane.isReflective());
}

TEST_F(ShapeBasics, ShapeDefaultMaterial)
{
    // specifying no material sets to default when constructing shape
    auto mat = s.getMaterial();
    EXPECT_EQ(mat.colour, Colour(1, 1, 1));
    EXPECT_DOUBLE_EQ(mat.ambient, 0.1);
    EXPECT_DOUBLE_EQ(mat.diffuse, 0.9);
    EXPECT_DOUBLE_EQ(mat.specular, 0.9);
    EXPECT_DOUBLE_EQ(mat.shininess, 200.0);
    EXPECT_DOUBLE_EQ(mat.reflectivity, 0.0);
    EXPECT_FALSE(s.isReflective());
    EXPECT_FALSE(s.isTransparent());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Abstract Shape Base Class
////////////////////////////////////////////////////////////////////////////////////////////////////
class ShapeBaseClass: public ShapeBasics
{
  protected:
    Group g1{}, g2{};
    Sphere sphere{};
};

TEST_F(ShapeBaseClass, HasDefaultTransformation)
{
    auto ident = TransformationMatrix::identity();
    EXPECT_EQ(s.getTransform(), ident);
}

TEST_F(ShapeBaseClass, CanAssignTransformations)
{
    auto T = Transform::translation(2., 3., 4.);
    s.setTransform(T);
    EXPECT_EQ(s.getTransform(), T);
}

TEST_F(ShapeBaseClass, HasDefaultMaterial)
{
    EXPECT_EQ(s.getMaterial(), (Material{}));
}

TEST_F(ShapeBaseClass, AssigningAMaterial)
{
    Material m{{}, 1.0};
    s.setMaterial(m);
    EXPECT_EQ(s.getMaterial(), m);
}

TEST_F(ShapeBaseClass, ScaledShapeIntersectedWithRay)
{
    Ray ray{ Point{0, 0, -5}, Vector{0, 0, 1} };
    s.setTransform(Transform::scale(2., 2., 2.));
    auto xs = s.intersect(ray);
    EXPECT_EQ(s.objectRay.getOrigin(), (Point{0, 0, -2.5}));
    EXPECT_EQ(s.objectRay.getDirection(), (Vector{0, 0, 0.5}));
}

TEST_F(ShapeBaseClass, TranslatedShapeIntersectedWithRay)
{
    Ray ray{ Point{0, 0, -5}, Vector{0, 0, 1} };
    s.setTransform(Transform::translation(5., 0., 0.));
    auto xs = s.intersect(ray);
    EXPECT_EQ(s.objectRay.getOrigin(), (Point{-5, 0, -5}));
    EXPECT_EQ(s.objectRay.getDirection(), (Vector{0, 0, 1}));
}

TEST_F(ShapeBaseClass, NormalOnTranslatedShape)
{
    s.setTransform(Transform::translation(0., 1., 0.));
    auto n = s.normalAt( Point{0, 1.70711, -0.70711} );
    EXPECT_EQ(n, Vector(0, 0.70711, -0.70711));
}

TEST_F(ShapeBaseClass, NormalOnTransformedShape)
{
    s.setTransform(Transform::scale(1.0, 0.5, 1.0)
                   * Transform::rotateZ(PI / 5.0));
    auto n = s.normalAt( Point{0.0, HALF_SQRT_2, -HALF_SQRT_2} );
    EXPECT_EQ(n, Vector(0, 0.97014, -0.24254));
}

TEST_F(ShapeBaseClass, WorldPointToObjectSpace)
{
    // convert a point in world space to shape/object space, considering
    //  any and all parent group objects between the two spaces
    Sphere s{};
    g1.setTransform(Transform::rotateY(HALF_PI));
    g2.setTransform(Transform::scale(2., 2., 2.));
    g1.addChild(&g2);
    s.setTransform(Transform::translation(5., 0., 0.));
    g2.addChild(&s);
    auto p = s.worldToObject(Point{ -2, 0, -10 });
    EXPECT_EQ(p, Point(0, 0, -1));
}

TEST_F(ShapeBaseClass, ObjectNormalToWorldSpace)
{
    // convert a normal in object space, back to world space, considering
    //  any and all parent group objects between the two spaces
    g1.setTransform(Transform::rotateY(HALF_PI));
    g2.setTransform(Transform::scale(1., 2., 3.));
    g1.addChild(&g2);
    Sphere s{};
    s.setTransform(Transform::translation(5., 0., 0.));
    g2.addChild(&s);
    auto n = s.normalToWorld(Vector{ THIRD_SQRT_3, THIRD_SQRT_3, THIRD_SQRT_3 });
    EXPECT_EQ(n, Vector(0.2857, 0.4286, -0.8571));
}

TEST_F(ShapeBaseClass, NormalOnChildObjects)
{
    // finding the normal on child objects which belong to a group shape
    // considering transformations on both children and parents
    g1.setTransform(Transform::rotateY(HALF_PI));
    g2.setTransform(Transform::scale(1., 2., 3.));
    g1.addChild(&g2);
    Sphere s{};
    s.setTransform(Transform::translation(5., 0., 0.));
    g2.addChild(&s);
    auto n = s.normalAt(Point{ 1.7321, 1.1547, -5.5774 });
    EXPECT_EQ(n, Vector(0.2857, 0.4286, -0.8571));
}


