#
#   RAYTRACER - LIBRARY TARGET
#

#
#   Library target
#
add_library(raytracer
        renderer/canvas.cpp
        renderer/colour.cpp
        renderer/intersection.cpp
        renderer/ray.cpp
        renderer/renderer.cpp
        renderer/job_finalizer.cpp
        renderer/job_scheduler.cpp
        shapes/cone.cpp
        shapes/csg.cpp
        shapes/cube.cpp
        shapes/cylinder.cpp
        shapes/group.cpp
        shapes/triangle.cpp
        shapes/shape.cpp
        shapes/sphere.cpp
        shapes/plane.cpp
        environment/camera.cpp
        environment/lighting.cpp
        environment/world.cpp
        materials/material.cpp
        materials/patterns.cpp
        materials/textures.cpp
        math/tuples.cpp
        math/matrix.cpp
        math/matrix_2d.cpp
        utils/obj_parser.cpp
        utils/utils.cpp
        logging/logging.cpp
)
add_library(raytracer::raytracer ALIAS raytracer)
if (BUILD_TESTING)
    target_compile_definitions(raytracer PRIVATE IS_TEST_SUITE=1) # IS_TEST_SUITE macro enabled
endif()

#
#   Include paths
#
target_include_directories(raytracer
        # public interface
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
            $<INSTALL_INTERFACE:include>
        # internal only header/src
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)

#
#   C++ options for public library target
#
target_compile_features(raytracer PUBLIC cxx_std_23)
if (MSVC)
    target_compile_options(raytracer PRIVATE /W4 /permissive-)
else()
    target_compile_options(raytracer PRIVATE -Wall -Wextra -Wpedantic)
endif()

#
#   Link deps
#
target_link_libraries(raytracer
        PUBLIC
            spdlog::spdlog
            $<$<BOOL:${MINGW}>:ws2_32>
        # PRIVATE
            # any internal stuff
)

#
#   Library export props
#
set_target_properties(raytracer PROPERTIES
        EXPORT_NAME raytracer               # cmake import/export name
        VERSION     ${PROJECT_VERSION}
        SOVERSION   ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "raytracer"             # name of actual binary file
)

#
#   Installation rules
#
if (INSTALL_LIBRARY)
    # todo!!!
endif()