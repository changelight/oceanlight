#include <iostream>
#include <config.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/util.hpp>
#include "args.hpp"
#include "engine.hpp"

int main(int argc, char **argv)
{
    int rv;

    struct oceanlight_args_struct args = {};
    rv = oceanlight_parse_args(argc, argv, &args);
    if (rv == 0)
    {
        return EXIT_FAILURE;
    }

    if (args.quit_flag == true)
    {
        return EXIT_SUCCESS;
    }
    
    rv = oceanlight_glfw_init();
    if (rv == 0)
    {
        return EXIT_FAILURE;
    }
    
    GLFWwindow *window = oceanlight_create_window(args.width, args.height);
    if (window == NULL)
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    
    rv = oceanlight_vk_init();
    if (rv == 0)
    {
        return EXIT_FAILURE;
    }

    try
    {
        oceanlight_engine_run(window);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
