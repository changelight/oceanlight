project(liboceanlight VERSION 0.0.1 LANGUAGES CXX)
configure_file(config.h.in config.h)
include(CTest)
add_library(liboceanlight STATIC
            src/lol_version.cc src/lol_engine_init.cc src/lol_window.cc src/lol_engine.cc src/lol_engine_shutdown.cc
            src/lol_debug_messenger.cc src/lol_glfw_callbacks.cc src/lol_utility.cc src/lol_version.cc src/stb_impl.cc
            src/tinyobjloader_impl.cc)
set_target_properties(liboceanlight PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden CMAKE_VISIBILITY_INLINES_HIDDEN yes)
target_compile_definitions(liboceanlight PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENV{VK_LOADER_DEBUG} "all")

include(FetchContent)
FetchContent_Declare(
    gsl
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/gsl
    GIT_REPOSITORY https://github.com/microsoft/GSL
    GIT_TAG origin/main
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(gsl)

include(FetchContent)
FetchContent_Declare(
    glfw
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/glfw
    GIT_REPOSITORY https://github.com/glfw/glfw)
FetchContent_MakeAvailable(glfw)

set(GLM_ENABLE_CXX_20 ON)
FetchContent_Declare(
    glm
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/glm
    GIT_REPOSITORY https://github.com/g-truc/glm)
FetchContent_MakeAvailable(glm)
target_compile_definitions(glm PUBLIC GLM_FORCE_SILENT_WARNINGS)
target_compile_definitions(glm PUBLIC GLM_BUILD_TESTS=OFF)

FetchContent_Declare(
    stb
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/stb
    URL https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/external/stb/src/stb_image.h)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/external/stb/src")

FetchContent_Declare(
    tinyobjloader
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/tinyobjloader
    URL https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/release/tiny_obj_loader.h
    DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(tinyobjloader)
add_library(tinyobjloader INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/external/tinyobjloader/src/tiny_obj_loader.h)
target_include_directories(tinyobjloader INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/external/tinyobjloader/src")

FetchContent_Declare(
    tinygltf
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/tinygltf
    URL https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h
    DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(tinygltf)
add_library(tinygltf INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/external/tinygltf/src/tiny_gltf.h)
target_include_directories(tinygltf INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/external/tinygltf/src")

FetchContent_Declare(
    googletest
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG origin/main)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
target_compile_options(gtest PRIVATE "-Wno-language-extension-token")
target_compile_definitions(gtest PUBLIC GTEST_LINKED_AS_SHARED_LIBRARY=0)
target_compile_definitions(gtest PUBLIC GTEST_CREATE_SHARED_LIBRARY=0)

find_package(Vulkan REQUIRED)

set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_DST_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_DST_DIR})
file(GLOB SHADERS ${SHADER_SRC_DIR}/*.vert ${SHADER_SRC_DIR}/*.frag)
foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME_WE)
    add_custom_command(OUTPUT ${SHADER_DST_DIR}/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_DST_DIR}/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILENAME}")
    list(APPEND SPV_SHADERS ${SHADER_DST_DIR}/${FILENAME}.spv)
endforeach(SHADER IN LISTS SHADERS)
add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_dependencies(liboceanlight shaders)

target_include_directories(liboceanlight PUBLIC "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}" ${Vulkan_INCLUDE_DIRS})
target_link_libraries(liboceanlight PUBLIC ${Vulkan_LIBRARIES} Microsoft.GSL::GSL glfw glm::glm stb tinyobjloader tinygltf)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
