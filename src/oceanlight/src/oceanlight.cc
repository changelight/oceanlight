#include <iostream>
#include <liboceanlight/util.hpp>
#include <config.h>
#include <GLFW/glfw3.h>
#include "args.hpp"
#include "engine.hpp"

int main(int argc, char **argv)
{
    int rv;
    parse_args(argc, argv);
    
    rv = glfw_init();

    if (!rv)
        return EXIT_FAILURE;
    
    GLFWwindow *window = create_window();
    if (!window)
    {
        return EXIT_FAILURE;
    }

    glfw_set_callbacks(window);

    run(window);

    glfwDestroyWindow(window);

    return EXIT_SUCCESS;
}
