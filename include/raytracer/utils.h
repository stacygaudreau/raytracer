#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// Some consts to keep code less noisy
constexpr double EPSILON{ 0.00001 };    // for surface normal offsets
constexpr double EPSILON_FP{ 0.0001 }; // for floating point numbers
constexpr double PI = std::numbers::pi;
constexpr double TWO_PI = 2. * PI;
constexpr double HALF_PI = PI / 2.;
constexpr double THIRD_PI = PI / 3.;
constexpr double QUARTER_PI = PI / 4.;
constexpr double SIXTH_PI = PI / 6.;
constexpr double SQRT_2 = std::numbers::sqrt2;
constexpr double SQRT_3 = std::numbers::sqrt3;
constexpr double THIRD_SQRT_3 = SQRT_3 / 3.;
constexpr double HALF_SQRT_2 = SQRT_2 / 2.;
constexpr double INF = std::numeric_limits<double>::infinity();


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Approximate equivalence for doubles. Handles float errors (via Epsilon).
inline bool APPROX_EQ(const double a, const double b)
{
    return std::abs(a - b) < EPSILON_FP;
}

/// @brief Approximately zero, for doubles.
inline bool APPROX_ZERO(const double a)
{
    return APPROX_EQ(a, 0.0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Utils
{
    /// @brief Swap two elements in place.
    template<typename T = double>
    inline void swap(T& a, T& b)
    {
        const T temp = a;
        a = b;
        b = temp;
    }

    /// @brief Split a std::string by a given delimiter.
    /// @details https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
    std::vector<std::string> split(const std::string &text, char sep);
    /// @brief Verify that a string is a double.
    bool isDouble(const std::string& s);
};
}
