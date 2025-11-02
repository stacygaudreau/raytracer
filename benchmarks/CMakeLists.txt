#
#   RAYTRACER - BENCHMARK SUITE
#

#
#   Fetch Google Benchmark
#   ref: https://github.com/google/benchmark/tree/main?tab=readme-ov-file#usage-with-cmake
#
FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.9.4
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)   # disable tests on bench
FetchContent_MakeAvailable(googlebenchmark)

#
#   BenchmarkSuite executable
#
add_executable(BenchmarkSuite
        bench_examples.cpp
)

target_link_libraries(BenchmarkSuite
        PRIVATE
            raytracer::raytracer
            benchmark::benchmark
            benchmark::benchmark_main
)

target_compile_features(BenchmarkSuite PUBLIC cxx_std_23)
# disable asserts
target_compile_definitions(BenchmarkSuite PRIVATE NDEBUG)

#
#   Optimize for benchmarking
#     - handles multi-config (MSVC/Xcode) and single-config (makefile/Ninja) Cmake
#        build generator types
#     - apply aggressive compiler optimizations regardless of debug/release/etc
#        being selected
#
if (CMAKE_CONFIGURATION_TYPES)
    # multi config
    if (MSVC)
        target_compile_options(BenchmarkSuite PRIVATE
                $<$<CONFIG:Debug>:/O2>
                $<$<CONFIG:RelWithDebInfo>:/O2>
                $<$<CONFIG:Release>:/O2>
                $<$<CONFIG:MinSizeRel>:/O2>
        )
    else()
        target_compile_options(BenchmarkSuite PRIVATE
                $<$<CONFIG:Debug>:-O3>
                $<$<CONFIG:RelWithDebInfo>:-O3>
                $<$<CONFIG:Release>:-O3>
                $<$<CONFIG:MinSizeRel>:-O3>
        )
    endif()
else()
    # single config
    if (MSVC)
        # /O2 ~ optimize for speed
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(BenchmarkSuite PRIVATE /O2)
        endif()
    else()
        # -O3 is aggressive optimization
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(BenchmarkSuite PRIVATE -O3)
        endif()
    endif()
endif()
