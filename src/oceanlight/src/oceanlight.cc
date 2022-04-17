#include <iostream>
#include <config.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/util.hpp>
#include "args.hpp"
#include "engine.hpp"

int main(int argc, char **argv)
{
    try
    {
        int rv;

        struct oceanlight_args args;
        oceanlight_parse_args(argc, argv, &args);

        if (args.exit_flag == true)
        {
            return EXIT_SUCCESS;
        }

        oceanlight_engine_instance engine;
        oceanlight_window window(args.width, args.height);
        
        engine.init();
        engine.run(window.window_pointer);
    }

    catch (const std::exception &e)
    {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
