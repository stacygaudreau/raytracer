#pragma once

#include <vector>
#include <algorithm>
#include "raytracer/renderer/colour.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class Matrix2D
{
  public:
    Matrix2D(size_t width, size_t height);
    size_t getWidth() const;
    size_t getHeight() const;
    bool isBlank();
    T get(size_t x, size_t y) const;
    void set(size_t x, size_t y, T value);
    const std::vector<std::vector<T>>& getMatrix() const;
    std::vector<T> getCol(size_t x) const;
    /// Set every element of this matrix to some given value. Resets every element to the value.
    void setAllElementsTo(T value);

  private:
    size_t width, height;
    std::vector<std::vector<T>> matrix;
};
}