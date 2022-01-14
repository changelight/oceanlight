configure_file(config.h.in config.h)
target_include_directories(oceanlight PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(oceanlight PRIVATE liboceanlight ${Vulkan_LIBRARIES} cxxopts glfw glm)
target_sources(oceanlight PRIVATE src/oceanlight.cc src/engine.cc src/args.cc)
