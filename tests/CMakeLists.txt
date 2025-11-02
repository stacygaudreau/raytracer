#
#   RAYTRACER - TEST SUITE TARGET
#

#
#   Fetch Google Test
#   ref: https://google.github.io/googletest/quickstart-cmake.html
#
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
)
# for windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#
#   TestSuite executable
#
add_executable(TestSuite
        test_camera.cpp
        test_canvas.cpp
        test_colours.cpp
        test_csg_set_operations.cpp
        test_cubes.cpp
        test_cylinder_and_cones.cpp
        test_groups.cpp
        test_lighting.cpp
        test_materials.cpp
        test_matrix.cpp
        test_matrix_2d.cpp
        test_parser.cpp
        test_pattern.cpp
        test_planes.cpp
        test_rays.cpp
        test_renderer.cpp
        test_shapes.cpp
        test_spheres.cpp
        test_textures.cpp
        test_triangles.cpp
        test_tuples.cpp
        test_world.cpp
        # delete this one later..
        test_log_async.cpp
)

target_link_libraries(TestSuite
        PRIVATE
            raytracer::raytracer
            GTest::gtest
            GTest::gtest_main
)

target_compile_features(TestSuite PUBLIC cxx_std_23)

# CTest
include(GoogleTest)
gtest_discover_tests(TestSuite)