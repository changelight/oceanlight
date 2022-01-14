cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
cmake_policy(SET CMP0076 NEW) # NEW: The target_sources() command converts relative paths to absolute.
cmake_policy(SET CMP0048 NEW) # NEW: The project() command manages VERSION variables.
cmake_policy(SET CMP0079 NEW) # NEW: target_link_libraries() allows use with targets in other directories.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Oceanlight VERSION 0.0.1 LANGUAGES CXX)

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

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

add_executable(oceanlight)
add_subdirectory(src)
