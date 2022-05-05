cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Oceanlight VERSION 0.0.1 LANGUAGES CXX)
include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    cxxopts
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts)
FetchContent_MakeAvailable(cxxopts)

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
include_directories(${Vulkan_INCLUDE_DIRS})

add_executable(oceanlight)
add_subdirectory(src)
