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

add_executable(oceanlight)
add_subdirectory(src)
