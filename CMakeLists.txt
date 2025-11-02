#
#   RAYTRACER
#   a 3d raytracing rendering library
#
cmake_minimum_required(VERSION 4.0)
project(raytracer
        VERSION 0.5.0
        DESCRIPTION "Raytracer"
        HOMEPAGE_URL "https://stacygaudreau.com"
        LANGUAGES CXX
)

#
#   Options
#
option(BUILD_DEMO "Build demo application" ON)
option(BUILD_TESTING "Build unit test suite" ON)
option(BUILD_BENCHMARKS "Build benchmark suite" ON)
option(INSTALL_LIB "System library install" ON)

#
#   Compiler/C++ config
#
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
#   Fetch common dependencies
#
include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.16.0
)
FetchContent_MakeAvailable(spdlog)

#
#   Subdirectories
#
add_subdirectory(src)
if (BUILD_DEMO)
    add_subdirectory(demo)
endif()
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

#
#   Install library
#
if (INSTALL_LIB)
    # todo!!
endif()
