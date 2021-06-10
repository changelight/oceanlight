project(liboceanlight VERSION 0.0.1 LANGUAGES CXX)
configure_file(config.h.in config.h)
add_library(liboceanlight STATIC src/util.cc)
target_include_directories(liboceanlight PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}")