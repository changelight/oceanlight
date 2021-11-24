cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
cmake_policy(SET CMP0076 NEW) # NEW: The target_sources() command converts relative paths to absolute.
cmake_policy(SET CMP0048 NEW) # NEW: The project() command manages VERSION variables.
cmake_policy(SET CMP0079 NEW) # NEW: target_link_libraries() allows use with targets in other directories.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Oceanlight VERSION 0.0.1 LANGUAGES CXX)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.76 REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

add_executable(oceanlight)
add_subdirectory(src)
