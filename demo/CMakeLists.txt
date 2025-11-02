#
#   RAYTRACER - DEMO APP
#

#
#   RunDemo executable
#
add_executable(RunDemo
        main.cpp
)

target_link_libraries(RunDemo
        PRIVATE
        raytracer::raytracer
)

target_compile_features(RunDemo PUBLIC cxx_std_23)

