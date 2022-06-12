#include <iostream>
#include <cxxopts.hpp>
#include <liboceanlight/util.hpp>
#include <config.h>
#include "args.hpp"

void oceanlight::parse_args(int argc, char** argv, struct oceanlight::args& args)
{
    try
    {
        cxxopts::Options options("oceanlight", "Chase your star");
        options.add_options()
            ("h,help", "Produce help message")
            ("v,version", "Print version information")
            ("x,width", "Set window width in pixels", cxxopts::value<int>())
            ("y,height", "Set window height in pixels", cxxopts::value<int>());

        auto result {options.parse(argc, argv)};

        if (result.count("help"))
        {
            std::cout << options.help();
            args.exit_flag = true;
            return;
        }

        if (result.count("version"))
        {
            std::cout << PROJECT_NAME " Ver. " PROJECT_VER "\n" << liboceanlight::version_string() << std::endl;
            args.exit_flag = true;
            return;
        }

        if (result.count("width"))
            args.width = result["width"].as<int>();

        if (result.count("height"))
            args.height = result["height"].as<int>();
    }

    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }

    catch (...)
    {
        std::cerr << "Error: unknown error" << std::endl;
        throw;
    }
}
