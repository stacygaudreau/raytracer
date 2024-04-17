file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
# all set to public for now but as the project grows it may make
# sense to separate public interface from private
target_include_directories(${PROJECT_NAME}
    PRIVATE
    # where the library looks for its own headers
    ${PROJECT_SOURCE_DIR}/source
    PUBLIC
    # where root project looks for headers
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>
    # where external projects look for headers
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)