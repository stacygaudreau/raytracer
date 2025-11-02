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
        test_class.cpp
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