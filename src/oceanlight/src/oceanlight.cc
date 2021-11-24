#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <liboceanlight/util.hpp>
#include <config.h>
#include "args.hpp"
#include "engine.hpp"

int main(int argc, char **argv)
{
    int rv;
    bool quit_flag = false;
    rv = parse_args(argc, argv, &quit_flag);

    if (!rv)
    {
        return EXIT_FAILURE;
    }

    if (quit_flag)
        return EXIT_SUCCESS;
    
    rv = glfw_init();

    if (!rv)
        return EXIT_FAILURE;
    
    GLFWwindow *window = create_window();
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

    return EXIT_SUCCESS;
}
