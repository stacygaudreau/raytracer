#include <iostream>

#include <raytracer/class.hpp>

int main() {
    auto sc = SomeClass{};
    auto z = sc(1, 2);
    spdlog::warn("1 + 2 = {}", z);
    std::cout << __FILE_NAME__ << "::" << __FUNCTION__ << "::()\n";
    SomeClass::say_hi();
}