#include "gtest/gtest.h"
#include "raytracer/math/matrix.hpp"
#include "raytracer/math/tuples.hpp"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
class MatrixBasics : public ::testing::Test
{
  protected:
    Matrix<double, 4> M4 = Matrix<double, 4> ({
        {
            { 1., 2., 3., 4. },
            { 5.5, 6.5, 7.5, 8.5 },
            { 9, 10, 11, 12 },
            { 13.5, 14.5, 15.5, 16.5 }
        }
    });
    Matrix<double, 3> M3 = Matrix<double, 3>({
        {
            {-3., 5, 0},
            {1., -2., -7.},
            {0., 1., 1.}
        }
    });
    Matrix<double, 2> M2 = Matrix<double, 2>({
        {
            {-3., 5},
            {1., -2.}
        }
    });
    Matrix<double, 4> A = Matrix<double, 4> ({
         {
             { 1., 2., 3., 4. },
             { 5., 6., 7., 8. },
             { 9., 8., 7., 6. },
             { 5., 4., 3., 2. }
        }
    });

    Matrix<double, 4> B = Matrix<double, 4> ({
         {
             { 1., 2., 3., 4. },
             { 5., 6., 7., 8. },
             { 9., 8., 7., 6. },
             { 5., 4., 3., 2. }
        }
    });

    Matrix<double, 4> C = Matrix<double, 4> ({
         {
             { 2., 3., 4., 5. },
             { 6., 7., 8., 9. },
             { 8., 7., 6., 5. },
             { 4., 3., 2., 1. }
        }
    });
};

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(MatrixBasics, Matrix4x4IsConstructed)
{
    EXPECT_DOUBLE_EQ(M4(0, 0), 1.);
    EXPECT_DOUBLE_EQ(M4(0, 3), 4.);
    EXPECT_DOUBLE_EQ(M4(1, 0), 5.5);
    EXPECT_DOUBLE_EQ(M4(1, 2), 7.5);
    EXPECT_DOUBLE_EQ(M4(2, 2), 11.);
    EXPECT_DOUBLE_EQ(M4(3, 0), 13.5);
    EXPECT_DOUBLE_EQ(M4(3, 2), 15.5);
}

TEST_F(MatrixBasics, Matrix3x3IsConstructed)
{
    EXPECT_DOUBLE_EQ(M3(0, 0), -3.);
    EXPECT_DOUBLE_EQ(M3(1, 1), -2.);
    EXPECT_DOUBLE_EQ(M3(2, 2), 1.);
}

TEST_F(MatrixBasics, Matrix2x2IsConstructed)
{
    EXPECT_DOUBLE_EQ(M2(0, 0), -3.);
    EXPECT_DOUBLE_EQ(M2(0, 1), 5.);
    EXPECT_DOUBLE_EQ(M2(1, 0), 1.);
    EXPECT_DOUBLE_EQ(M2(1, 1), -2.);
}

TEST_F(MatrixBasics, MatrixEqualityIdenticalIsTrue)
{
    EXPECT_EQ(A, B);
}

TEST_F(MatrixBasics, MatrixEqualityDifferentIsFalse)
{
    EXPECT_NE(A, C);
}

TEST_F(MatrixBasics, MatrixMultiplication)
{
    auto B = Matrix<double, 4> ({
        {
            { -2., 1., 2., 3. },
            { 3., 2., 1., -1. },
            { 4., 3., 6., 5. },
            { 1., 2., 7., 8. }
        }
    });

    auto expected = Matrix<double, 4> ({
        {
            { 20., 22., 50., 48. },
            { 44., 54., 114., 108. },
            { 40., 58., 110., 102. },
            { 16., 26., 46., 42. }
        }
    });

    EXPECT_EQ(A * B, expected);
}

TEST_F(MatrixBasics, Matrix4x4MultipliedByTuple)
{
    auto A = Matrix<double, 4> ({
        {
            { 1., 2., 3., 4. },
            { 2., 4., 4., 2 },
            { 8., 6., 4., 1. },
            { 0., 0., 0., 1. }
        }
    });
    auto b = Tuple{1., 2., 3., 1.};
    auto expected = Tuple{ 18., 24., 33., 1. };
    auto res = A * b;
    EXPECT_EQ(res, expected);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class MatrixAdvanced : public ::testing::Test
{
  protected:

    Matrix<double, 4> I4 = Matrix<double, 4> ({
        {
            {1., 0., 0., 0.},
            {0., 1., 0., 0.},
            {0., 0., 1., 0.},
            {0., 0., 0., 1.},
        }
    });
    Matrix<double, 3> A = Matrix<double, 3>({
                      {
                          {3., 5., 0.},
                          {2., -1., -7.},
                          {6., -1., 5.}
        }
    });
};


TEST_F(MatrixAdvanced, IdentityMatrix)
{
    auto I2 = Matrix<double, 2> ({
        {
            {1., 0.},
            {0., 1.}
        }
    });
    auto I3 = Matrix<double, 3> ({
        {
            {1., 0., 0.},
            {0., 1., 0.},
            {0., 0., 1.},
        }
    });
    auto I4 = Matrix<double, 4> ({
        {
            {1., 0., 0., 0.},
            {0., 1., 0., 0.},
            {0., 0., 1., 0.},
            {0., 0., 0., 1.},
        }
    });
    auto M2 = Matrix<double, 2>::identity();
    auto M3 = Matrix<double, 3>::identity();
    auto M4 = Matrix<double, 4>::identity();
    EXPECT_EQ(M2, I2);
    EXPECT_EQ(M3, I3);
    EXPECT_EQ(M4, I4);
}

TEST_F(MatrixAdvanced, TransposingMatrices)
{
    auto A = Matrix<double, 4>({
        {
            {0., 9., 3., 0.},
            {9., 8., 0., 8.},
            {1., 8., 5., 3.},
            {0., 0., 5., 8.}
        }
    });
    auto expected = Matrix<double, 4>({
        {
            {0., 9., 1., 0.},
            {9., 8., 8., 0.},
            {3., 0., 5., 5.},
            {0., 8., 3., 8.}
        }
    });
    EXPECT_EQ(A.transposed(), expected);
}

TEST_F(MatrixAdvanced, TransposingIdentityMatrix)
{
    // transposing the identity matrix *IS* the identity matrix
    EXPECT_EQ(I4, I4.transposed());
}

TEST_F(MatrixAdvanced, Determinant2x2)
{
    // calculating the determinant of 2x2 matrix
    auto A = Matrix<double, 2>({
        {
            {1., 5.},
            {-3., 2.}
        }
    });
    EXPECT_DOUBLE_EQ(A.determinant(), 17.);
}

TEST_F(MatrixAdvanced, SubmatrixOf3x3Is2x2)
{
    // the submat. of a 3x3 results in a 2x2 matrix
    auto A = Matrix<double, 3>({
        {
            {1., 5., 0.},
            {-3., 2., 7.},
            {0., 6., -3.}
        }
    });
    auto sub = Matrix<double, 2>({
        {
            {-3., 2.},
            {0., 6.}
        }
    });
    EXPECT_EQ(A.subMatrix(0, 2), sub);
}

TEST_F(MatrixAdvanced, SubmatrixOf4x4is3x3)
{
    // the submat. of a 4x4 results in a 3x3 matrix
    auto A = Matrix<double, 4>({
        {
            {-6., 1., 1., 6.},
            {-8., 5., 8., 6.},
            {-1., 0., 8., 2.},
            {-7., 1., -1., 1.}
        }
    });
    auto sub = Matrix<double, 3>({
        {
            {-6., 1., 6.},
            {-8., 8., 6.},
            {-7., -1., 1.}
        }
    });
    EXPECT_EQ(A.subMatrix(2, 1), sub);
}

TEST_F(MatrixAdvanced, MinorOf3x3Matrix)
{
    // the minor of an element at i,j is
    // the det. of the submat at i,j
    auto B = A.subMatrix(1, 0);
    EXPECT_DOUBLE_EQ(B.determinant(), 25.);
    EXPECT_DOUBLE_EQ(A.minor(1, 0), 25.);
}

TEST_F(MatrixAdvanced, CofactorOf3x3Matrix)
{
    EXPECT_DOUBLE_EQ(A.minor(0, 0), -12.);
    EXPECT_DOUBLE_EQ(A.cofactor(0, 0), -12.);
    EXPECT_DOUBLE_EQ(A.minor(1, 0), 25.);
    EXPECT_DOUBLE_EQ(A.cofactor(1, 0), -25.);
}

TEST_F(MatrixAdvanced, Determinant3x3)
{
    auto D3 = Matrix<double, 3>({
        {
            {1., 2., 6.},
            {-5., 8., -4.},
            {2., 6., 4.}
        }
    });
    EXPECT_DOUBLE_EQ(D3.cofactor(0, 0), 56.);
    EXPECT_DOUBLE_EQ(D3.cofactor(0, 1), 12.);
    EXPECT_DOUBLE_EQ(D3.cofactor(0, 2), -46.);
    EXPECT_DOUBLE_EQ(D3.determinant(), -196.);
}

TEST_F(MatrixAdvanced, Determinant4x4)
{
    auto D4 = Matrix<double, 4>({
        {
            {-2., -8., 3., 5},
            {-3., 1., 7., 3.},
            {1., 2., -9., 6.},
            {-6., 7., 7., -9.}
        }
    });
    EXPECT_DOUBLE_EQ(D4.cofactor(0, 0), 690.);
    EXPECT_DOUBLE_EQ(D4.cofactor(0, 1), 447.);
    EXPECT_DOUBLE_EQ(D4.cofactor(0, 2), 210.);
    EXPECT_DOUBLE_EQ(D4.cofactor(0, 3), 51.);
    EXPECT_DOUBLE_EQ(D4.determinant(), -4071.);
}

TEST_F(MatrixAdvanced, InvertibleMatrixIsInvertible)
{
    auto A = Matrix<double, 4>({
        {
            {6., 4., 4., 4.},
            {5., 5., 7., 6.},
            {4., -9., 3., -7.},
            {9., 1., 7., -6.,}
        }
    });
    EXPECT_EQ(A.determinant(), -2120.);
    EXPECT_TRUE(A.isInvertible());
}

TEST_F(MatrixAdvanced, NonInvertibleMatrixIsNotInvertible)
{
    auto A = Matrix<double, 4>({
        {
            {-4., 2., -2., -3.},
            {9., 6., 2., 6.},
            {0., -5., 1., -5.},
            {0., 0., 0., 0.,}
        }
    });
    EXPECT_EQ(A.determinant(), 0.);
    EXPECT_FALSE(A.isInvertible());
}

TEST_F(MatrixAdvanced, InverseOf4x4Matrix)
{
    // calculate the inverse of a 4x4 matrox
    auto A = Matrix<double, 4>({
        {
            {-5., 2., 6., -8.},
            {1., -5., 1., 8.},
            {7., 7., -6., -7.},
            {1., -3., 7., 4.,}
        }
    });
    auto B = A.inverse();
    EXPECT_DOUBLE_EQ(A.determinant(), 532.);
    EXPECT_DOUBLE_EQ(A.cofactor(2, 3), -160.);
    EXPECT_DOUBLE_EQ(B(3, 2), -160. / 532.);
    EXPECT_DOUBLE_EQ(A.cofactor(3, 2), 105.);
    EXPECT_DOUBLE_EQ(B(2, 3), 105. / 532.);
    auto inverse = Matrix<double, 4>({
        {
            { .21805,   .45113,   .24060, -.04511},
            {-.80827, -1.45677,  -.44361,  .52068},
            {-.07895,  -.22368,  -.05263,  .19737},
            {-.52256,  -.81391,  -.30075,  .30639}
        }
    });
    EXPECT_EQ(B, inverse);
}

TEST_F(MatrixAdvanced, InverseOfAnother4x4)
{
    auto A = Matrix<double, 4>({
        {
            {8., -5., 9., 2.},
            {7., 5., 6., 1.},
            {-6., 0., 9., 6.},
            {-3., 0., -9., -4.}
        }
    });
    auto B = A.inverse();
    auto inverse = Matrix<double, 4>({
        {
            {-.15385, -.15385, -.28205, -.53846},
            {-.07692, .12308, .02564, .03077},
            {.35897, .35897, .43590, .92308},
            {-.69231, -.69231, -.76923, -1.92308}
        }
    });
    EXPECT_EQ(B, inverse);
}

TEST_F(MatrixAdvanced, InverseOfThird4x4Matrix)
{
    auto A = Matrix<double, 4>({
        {
            {9., 3., 0., 9.},
            {-5., -2., -6., -3.},
            {-4., 9., 6., 4.},
            {-7., 6., 6., 2.}
        }
    });
    auto B = A.inverse();
    auto inverse = Matrix<double, 4>({
        {
            {-.04074, -.07778, 0.14444, -0.22222},
            {-.07778, .03333, .36667, -.33333},
            {-.02901, -.14630, -.10926, .12963},
            {.17778, .06667, -.26667, .33333}
        }
    });
    EXPECT_EQ(B, inverse);
}

TEST_F(MatrixAdvanced, MultiplyingAProductByItsInverse)
{
    auto A = Matrix<double, 4>({
        {
            {3., -9., 7., 3.},
            {3., -8., 2., -9.},
            {-4., 4., 4., 1.},
            {-6., 5., -1., 1.}
        }
    });
    auto B = Matrix<double, 4>({
        {
            {8., 2., 2., 2.},
            {3., -1., 7., 0.},
            {7., 0., 5., 4.},
            {6., -2., 0., 5.}
        }
    });
    auto C = A * B;
    EXPECT_EQ(C * B.inverse(), A);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// MatrixTransformations
////////////////////////////////////////////////////////////////////////////////////////////////////

class MatrixTransformations : public ::testing::Test
{
  protected:
};

TEST_F(MatrixTransformations, MultiplyingByTranslationMatrix)
{
    // multiplying by a transformation matrix
    auto transform = Transform::translation(5., -3., 2.);
    auto inv = transform.inverse();
    auto p = Point(-3., 4., 5.);
    EXPECT_EQ(inv * p, Point(-8., 7., 3.));
}

TEST_F(MatrixTransformations, TranslationDoesNotAffectVectors)
{
    // translating does not affect vectors thanks to w = 0
    auto transform = Transform::translation(5., -3., 2.);
    auto v = Vector(-3., 4., 5.);
    EXPECT_EQ(transform * v, v);
}


TEST_F(MatrixTransformations, ScalingAppliedToPoint)
{
    auto transform = Transform::scale(2., 3., 4.);
    auto p = Point(-4., 6., 8.);
    EXPECT_EQ(transform * p, Point(-8., 18., 32.));
}

TEST_F(MatrixTransformations, ScalingAppliedToVector)
{
    auto transform = Transform::scale(2., 3., 4.);
    auto v = Vector(-4., 6., 8.);
    EXPECT_EQ(transform * v, Vector(-8., 18., 32.));
}

TEST_F(MatrixTransformations, InverseOfScalingIsOppositeEffect)
{
    auto transform = Transform::scale(2., 3., 4.);
    auto inv = transform.inverse();
    auto v = Vector(-4., 6., 8.);
    EXPECT_EQ(inv * v, Vector(-2., 2., 2.));
}

TEST_F(MatrixTransformations, ReflectionIsScalingByNegativeVal)
{
    auto transform = Transform::scale(-1., 1., 1.);
    auto p = Point(2., 3., 4.);
    EXPECT_EQ(transform * p, Point(-2., 3., 4.));
}

TEST_F(MatrixTransformations, RotatePointAroundX)
{
    auto p = Point(0., 1., 0.);
    auto halfQuarter = Transform::rotateX(QUARTER_PI);
    auto fullQuarter = Transform::rotateX(HALF_PI);
    EXPECT_EQ(halfQuarter * p, Point(0, SQRT_2/2., SQRT_2/2.));
    EXPECT_EQ(fullQuarter * p, Point(0., 0., 1.));
}

TEST_F(MatrixTransformations, InverseOfPointRotatingAroundX)
{
    auto p = Point(0., 1., 0.);
    auto halfQuarter = Transform::rotateX(QUARTER_PI);
    auto inv = halfQuarter.inverse();
    EXPECT_EQ(inv * p, Point(0, SQRT_2/2., -SQRT_2/2.));
}

TEST_F(MatrixTransformations, RotatePointAroundY)
{
    auto p = Point(0., 0., 1.);
    auto halfQuarter = Transform::rotateY(QUARTER_PI);
    auto fullQuarter = Transform::rotateY(HALF_PI);
    EXPECT_EQ(halfQuarter * p, Point(SQRT_2/2., 0., SQRT_2/2.));
    EXPECT_EQ(fullQuarter * p, Point(1., 0., 0.));
}

TEST_F(MatrixTransformations, RotatePointAroundZ)
{
    auto p = Point(0., 1., 0.);
    auto halfQuarter = Transform::rotateZ(QUARTER_PI);
    auto fullQuarter = Transform::rotateZ(HALF_PI);
    EXPECT_EQ(halfQuarter * p, Point(-SQRT_2/2., SQRT_2/2., 0.));
    EXPECT_EQ(fullQuarter * p, Point(-1., 0., 0.));
}

TEST_F(MatrixTransformations, ShearXProportionateToY)
{
    auto p = Point(2., 3., 4.);
    auto shear = Transform::shear(1., 0., 0., 0., 0., 0.);
    EXPECT_EQ(shear * p, Point(5., 3., 4.));
}

TEST_F(MatrixTransformations, ShearXProportionateToZ)
{
    auto p = Point(2., 3., 4.);
    auto shear = Transform::shear(0., 1., 0., 0., 0., 0.);
    EXPECT_EQ(shear * p, Point(6., 3., 4.));
}

TEST_F(MatrixTransformations, ShearYProportionateToX)
{
    auto p = Point(2., 3., 4.);
    auto shear = Transform::shear(0., 0., 1., 0., 0., 0.);
    EXPECT_EQ(shear * p, Point(2., 5., 4.));
}

TEST_F(MatrixTransformations, ShearYProportionateToZ)
{
    auto p = Point(2., 3., 4.);
    auto shear = Transform::shear(0., 0., 0., 1., 0., 0.);
    EXPECT_EQ(shear * p, Point(2., 7., 4.));
}

TEST_F(MatrixTransformations, ShearZProportionateToX)
{
    auto p = Point(2., 3., 4.);
    auto shear = Transform::shear(0., 0., 0., 0., 1., 0.);
    EXPECT_EQ(shear * p, Point(2., 3., 6.));
}

TEST_F(MatrixTransformations, ShearZProportionateToY)
{
    auto p = Point(2., 3., 4.);
    auto shear = Transform::shear(0., 0., 0., 0., 0., 1.);
    EXPECT_EQ(shear * p, Point(2., 3., 7.));
}

TEST_F(MatrixTransformations, TransformationsAppliedInSequence)
{
    auto p = Point(1., 0., 1.);
    auto A = Transform::rotateX(HALF_PI);
    auto B = Transform::scale(5., 5., 5.);
    auto C = Transform::translation(10., 5., 7.);
    auto p2 = A * p;
    EXPECT_EQ(p2, Point(1., -1., 0.));
    auto p3 = B * p2;
    EXPECT_EQ(p3, Point(5., -5., 0.));
    auto p4 = C * p3;
    EXPECT_EQ(p4, Point(15., 0., 7.));
}

TEST_F(MatrixTransformations, ChainingTransformationsAndApplying)
{
    // chained transformations must be applied in reverse order
    auto p = Point(1., 0., 1.);
    auto A = Transform::rotateX(HALF_PI);
    auto B = Transform::scale(5., 5., 5.);
    auto C = Transform::translation(10., 5., 7.);
    auto T = C * B * A;
    EXPECT_EQ(T * p, Point(15., 0., 7.));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// ViewTransformations
////////////////////////////////////////////////////////////////////////////////////////////////////
class ViewTransformations : public ::testing::Test
{
  protected:
};

TEST_F(ViewTransformations, DefaultViewOrientationMatrix)
{
    // the view transform matrix for the default world orientation (negative Z)
    auto from = Point{};
    auto to = Point{ 0, 0, -1 };
    auto up = Vector{ 0, 1, 0 };
    auto T = Transform::viewTransform(from, to, up);
    EXPECT_EQ(T, TransformationMatrix::identity());
}

TEST_F(ViewTransformations, ViewInPositiveZ)
{
    // view transform matrix looking in positive Z direction
    auto from = Point{};
    auto to = Point{ 0, 0, 1 };
    auto up = Vector{ 0, 1, 0 };
    auto T = Transform::viewTransform(from, to, up);
    // this is actually the same as scaling by a negative value (ie: reflecting)
    EXPECT_EQ(T, Transform::scale(-1., 1., -1.));
}

TEST_F(ViewTransformations, ViewMovesTheWorldNotEye)
{
    // it really does move the World, and not the "eye" (aka: "from" parameter)
    auto from = Point{ 0, 0, 8 };
    auto to = Point{ 0, 0, 0 };
    auto up = Vector{ 0, 1, 0 };
    auto T = Transform::viewTransform(from, to, up);
    // the world is therefore pushed 8 units away from the origin (eye)
    EXPECT_EQ(T, Transform::translation(0., 0., -8.));
}

TEST_F(ViewTransformations, ArbitraryView)
{
    // produces a matrix which is combination of shearing, scaling and translation
    auto from = Point{ 1., 3., 2. };
    auto to = Point{ 4., -2., 8. };
    auto up = Vector{ 1., 1., 0. };
    auto T = Transform::viewTransform(from, to, up);
    auto M = TransformationMatrix({
        {
            {-.50709, .50709, .67612, -2.36643},
            {.76772, .60609, .12122, -2.82843},
            {-.35857, .59761, -.71714, 0.0},
            {0.0, 0.0, 0.0, 1.0}
        }
    });
    EXPECT_EQ(T, M);
}

