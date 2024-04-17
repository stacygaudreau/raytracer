cmake_minimum_required(VERSION 3.27)
project(raytracer VERSION 0.5.0 DESCRIPTION "Raytracer Library" LANGUAGES CXX)

# compiler and build settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Wextra -Wpedantic")
set(CMAKE_VERBOSE_MAKEFILE on)
# release build if no build type is specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(source)  # raytracer library source

# test target with Gtest
file(GLOB TEST_SOURCES "tests/*.cpp")
# FetchContent to load and build googletest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# demo executable target
add_executable(demo demo/main.cpp)
target_link_libraries(demo PUBLIC ${PROJECT_NAME})

# declare and link tests target
enable_testing()
add_executable(
        tests
        ${TEST_SOURCES}
)
target_link_libraries(tests PRIVATE gtest_main PUBLIC ${PROJECT_NAME})
include(GoogleTest)
gtest_discover_tests(tests)

# todo: library and header installation