#pragma once

#include <spdlog/spdlog.h>

class SomeClass {
public:
    SomeClass() {
        spdlog::info("constructing SomeClass");
    }
    ~SomeClass() {
        spdlog::warn("destroying SomeClass");
    }
    static void say_hi();
    int operator()(int x, int y) const { return x + y; }
};