project(Oceanlight VERSION 0.0.1 LANGUAGES CXX)
configure_file(config.h.in config.h)
include(CTest)


include(FetchContent)
FetchContent_Declare(
    cxxopts
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts)
FetchContent_MakeAvailable(cxxopts)

add_executable(oceanlight)
target_compile_definitions(oceanlight PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)
target_include_directories(oceanlight PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}")
target_link_libraries(oceanlight PRIVATE cxxopts liboceanlight)
target_sources(oceanlight PRIVATE src/oceanlight.cc src/args.cc)
