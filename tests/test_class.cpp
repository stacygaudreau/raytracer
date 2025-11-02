#include <gtest/gtest.h>
#include <raytracer/class.hpp>

TEST(SomeClass, CallOperator) {
    auto sc = SomeClass{};
    EXPECT_EQ(sc(20, 10), 30);
}