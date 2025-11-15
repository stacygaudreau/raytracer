////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: Matrix
///     Provides square matrices (of n by n dimensions)
///     Stacy Gaudreau
///     18.06.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <numbers>

#include "raytracer/common/utils.hpp"
#include "raytracer/math/tuples.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T=double, size_t N=4>
class Matrix
{
  public:
    Matrix() = default;
    explicit Matrix(std::array<std::array<T, N>, N> initialValues);
    [[nodiscard]] size_t getSize() const;
    T& operator()(size_t row, size_t col);
    T operator()(size_t row, size_t col) const;
    /// Construct an identity matrix of the given matrix size.
    static Matrix<T, N> identity();
    /// Transposed version of this matrix. Returns a new, transposed matrix.
    Matrix<T, N> transposed() const;
    /// Compute the determinant of this matrix.
    T determinant() const;
    /// Derive a sub-matrix from this matrix.
    Matrix<T, N-1> subMatrix(size_t row, size_t col) const;
    /// Compute the minor of an element at row, col of this matrix.
    T minor(size_t row, size_t col) const;
    /// Calculate the cofactor of this matrix at some element's row, col
    T cofactor(size_t row, size_t col) const;
    /// True if this matrix can be inverted.
    [[nodiscard]] bool isInvertible() const;
    /// Return the inverse of this matrix.
    Matrix<T, N> inverse() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Equality of matrices
    friend bool operator==(const Matrix<T, N>& a, const Matrix<T, N>& b) {
        for (size_t row{}; row < a.getSize(); row++) {
            for (size_t col{}; col < a.getSize(); col++)
                if ( !APPROX_EQ(a(row, col), b(row, col)) ) return false;
        }
        return true;
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// Matrix multiplication
    friend Matrix<T, N> operator*(const Matrix<T, N>& A, const Matrix<T, N>& B) {
        auto X = Matrix<T, N>{};
        for(size_t row{}; row < A.getSize(); row++) {
            for (size_t col{}; col < A.getSize(); col++) {
                T element{};
                for (size_t i{}; i < A.getSize(); i++)
                    element += A(row, i) * B(i, col);
                X(row, col) = element;
            }
        }
        return X;
    };
    /// Multiply this 4x4 matrix with a tuple (Point() or Vector()).
    Tuple operator*(Tuple t) const {
        auto X = Tuple{};
        constexpr auto SIZE{4};
        for(size_t row{}; row < SIZE; row++) {
            for (size_t col{}; col < SIZE; col++) {
                T element{};
                for (size_t i{}; i < SIZE; i++)
                    element += M[row][i] * t(i);
                X(row) = element;
            }
        }
        return X;
    };

  private:
    std::array<std::array<T, N>, N> M{};
};

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
Matrix<T, N>::Matrix(std::array<std::array<T, N>, N> initialValues)
: M(initialValues)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
size_t Matrix<T, N>::getSize() const
{
    return M.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
T& Matrix<T, N>::operator()(size_t row, size_t col)
{
    return M[row][col];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
T Matrix<T, N>::operator()(size_t row, size_t col) const
{
    return M[row][col];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
Matrix<T, N> Matrix<T, N>::identity()
{
    Matrix<T, N> M{};
    for (size_t i{}; i < M.getSize(); i++)
        M(i, i) = 1.;
    return M;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
Matrix<T, N> Matrix<T, N>::transposed() const
{
    Matrix<T, N> t{};
    for (size_t row{}; row < N; row++) {
        for (size_t col{}; col < N; col++) {
           t(col, row) = M[row][col];
        }
    }
    return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
T Matrix<T, N>::determinant() const
{
    T det{};
    if constexpr (N == 2)
        det = M[0][0]*M[1][1] - M[0][1]*M[1][0];
    else {
        for (size_t col{}; col < M.size(); ++col)
            det += M[0][col] * cofactor(0, col);
    }

    return det;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
Matrix<T, N-1> Matrix<T, N>::subMatrix(size_t row, size_t col) const
{
    Matrix<T, N-1> sub{};
    std::vector<std::vector<T>> toCopy{};
    // make the toCopy vector from source matrix
    for (size_t r{}; r < N; ++r) {
        // make a new row vector
        std::vector<T> rowVec{};
        for (size_t c{}; c < N; ++c) {
            if (r != row && c != col)
                rowVec.push_back(M[r][c]);
        }
        // push it onto copy vector matrix if it belongs there
        if (r != row)
            toCopy.push_back(rowVec);
    }
    // now copy the vector into our new submatrix
    for (size_t r{}; r < toCopy.size(); ++r) {
        for (size_t c{}; c < toCopy.size(); ++c) {
            sub(r, c) = toCopy[r][c];
        }
    }
    return sub;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
T Matrix<T, N>::minor(size_t row, size_t col) const
{
    return subMatrix(row, col).determinant();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
T Matrix<T, N>::cofactor(size_t row, size_t col) const
{
    auto m = minor(row, col);
    return (row + col) % 2 == 0 ? m : -m;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
bool Matrix<T, N>::isInvertible() const
{
    return determinant() != 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, size_t N>
Matrix<T, N> Matrix<T, N>::inverse() const
{
    if (!isInvertible()) throw std::runtime_error("Matrix is not invertible.");
    auto M2 = Matrix<T, N>{};
    for (size_t row{}; row < N; ++row) {
        for (size_t col{}; col < N; ++col) {
            auto c = cofactor(row, col);
            M2(col, row) = c / determinant();
        }
    }
    return M2;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Matrix type aliases
using TransformationMatrix = Matrix<double, 4>;


//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Linear Transformation functions which aren't Matrix() class members.
namespace Transform
{

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Construct a 4x4 translation matrix with the given x, y, z.
template <typename T = double>
Matrix<T, 4> translation(T x, T y, T z)
{
    auto translation  = Matrix<T, 4>::identity();
    translation(0, 3) = x;
    translation(1, 3) = y;
    translation(2, 3) = z;
    return translation;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Construct a 4x4 scaling matrix with the given x y and z scale factors.
template <typename T = double>
Matrix<T, 4> scale(T x, T y, T z)
{
    auto scaling  = Matrix<T, 4>::identity();
    scaling(0, 0) = x;
    scaling(1, 1) = y;
    scaling(2, 2) = z;
    return scaling;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Construct a transformation matrix to rotate around the X-axis.
template <typename T = double>
Matrix<T, 4> rotateX(double angle)
{
    auto rotate  = Matrix<T, 4>::identity();
    rotate(1, 1) = std::cos(angle);
    rotate(1, 2) = -std::sin(angle);
    rotate(2, 1) = std::sin(angle);
    rotate(2, 2) = std::cos(angle);
    return rotate;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Construct a transformation matrix to rotate around the Y-axis.
template <typename T = double>
Matrix<T, 4> rotateY(double angle)
{
    auto rotate  = Matrix<T, 4>::identity();
    rotate(0, 0) = std::cos(angle);
    rotate(0, 2) = std::sin(angle);
    rotate(2, 0) = -std::sin(angle);
    rotate(2, 2) = std::cos(angle);
    return rotate;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Construct a transformation matrix to rotate around the Z-axis.
template <typename T = double>
Matrix<T, 4> rotateZ(T angle)
{
    auto rotate  = Matrix<T, 4>::identity();
    rotate(0, 0) = std::cos(angle);
    rotate(0, 1) = -std::sin(angle);
    rotate(1, 0) = std::sin(angle);
    rotate(1, 1) = std::cos(angle);
    return rotate;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Construct a transformation matrix to rotate around the Z-axis.
template <typename T = double>
Matrix<T, 4> shear(T xY, T xZ, T yX, T yZ, T zX, T zY)
{
    auto shear  = Matrix<T, 4>::identity();
    shear(0, 1) = xY;
    shear(0, 2) = xZ;
    shear(1, 0) = yX;
    shear(1, 2) = yZ;
    shear(2, 0) = zX;
    shear(2, 1) = zY;
    return shear;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Create a world view transformation matrix.
template <typename T = double>
Matrix<T, 4> viewTransform(Point from, Point to, Vector up)
{
    auto forward = (to - from).normalize();
    auto left = cross(forward, up.normalize());
    // the original up vector is only approximately up, which makes framing scenes easier
    // since it avoids manual calculations of the exact up direction to program in..
    auto trueUp = cross(left, forward);
    auto orientation = Matrix<T, 4>({
        {
            {left.x, left.y, left.z, 0.},
            {trueUp.x, trueUp.y, trueUp.z, 0.},
            {-forward.x, -forward.y, -forward.z, 0.},
            {0., 0., 0., 1.}
        }
    });
    // we transform by the "from" point to move the orientation into the scene
    return orientation * translation(-from.x, -from.y, -from.z);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
} // END namespace Transform
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Linear Algebra functions which don't (or can't) belong in any one class in particular
//////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Linear {

//////////////////////////////////////////////////////////////////////////////////////////////////////
} // END namespace Linear
//////////////////////////////////////////////////////////////////////////////////////////////////////

}
