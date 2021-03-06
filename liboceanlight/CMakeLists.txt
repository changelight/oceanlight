project(liboceanlight VERSION 0.0.1 LANGUAGES CXX)
configure_file(config.h.in config.h)
include(CTest)
add_library(liboceanlight STATIC src/engine.cc src/util.cc src/error.cc)
set_target_properties(liboceanlight PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden CMAKE_VISIBILITY_INLINES_HIDDEN yes)
target_compile_definitions(liboceanlight PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    glfw
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/glfw
    GIT_REPOSITORY https://github.com/glfw/glfw)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/glm
    GIT_REPOSITORY https://github.com/g-truc/glm)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    googletest
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG origin/main)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Vulkan REQUIRED)
target_include_directories(liboceanlight PUBLIC "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}" ${Vulkan_INCLUDE_DIRS})
target_link_libraries(liboceanlight PUBLIC ${Vulkan_LIBRARIES} glfw glm)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
