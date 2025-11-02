#
#   RAYTRACER - LIBRARY TARGET
#

#
#   Library target
#
add_library(raytracer
        camera.cpp
        canvas.cpp
        colours.cpp
        cone.cpp
        csg.cpp
        cube.cpp
        cylinder.cpp
        group.cpp
        intersection.cpp
        lighting.cpp
        materials.cpp
        matrix.cpp
        matrix_2d.cpp
        obj_parser.cpp
        patterns.cpp
        plane.cpp
        rays.cpp
        renderer.cpp
        shapes.cpp
        sphere.cpp
        textures.cpp
        triangle.cpp
        tuples.cpp
        utils.cpp
        world.cpp
)
add_library(raytracer::raytracer ALIAS raytracer)

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