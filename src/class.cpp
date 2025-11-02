#include "raytracer/class.hpp"

#include <iostream>

void SomeClass::say_hi() {
    std::cout << __FILE_NAME__ << "::" << __FUNCTION__ << "::()\n";
}
