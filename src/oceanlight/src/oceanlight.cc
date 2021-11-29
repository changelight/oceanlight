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

    struct args_struct args = {};
    rv = parse_args(argc, argv, &args);

    if (!rv)
    {
        return EXIT_FAILURE;
    }

    if (args.quit_flag)
        return EXIT_SUCCESS;
    
    rv = glfw_init();

    if (!rv)
        return EXIT_FAILURE;
    
    GLFWwindow *window = create_window(args.width, args.height);
    if (!window)
    {
        return EXIT_FAILURE;
    }

    glfw_set_callbacks(window);

    rv = vk_init();

    if (!rv)
        return EXIT_FAILURE;

    run(window);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
