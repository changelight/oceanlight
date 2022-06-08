#include <iostream>
#include <vector>
#include <config.h>
#include <liboceanlight/engine.hpp>
#include <liboceanlight/util.hpp>
#include "args.hpp"

int main(int argc, char **argv)
{
    try
    {
        struct oceanlight::args args;
        oceanlight::parse_args(argc, argv, args);

        if (args.exit_flag == true)
        {
            return EXIT_SUCCESS;
        }

        static liboceanlight::engine engine;
        liboceanlight::window window;
        
        engine.init();
        engine.run(window);
    }

    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
