#include <iostream>
#include <boost/program_options.hpp>
#include <liboceanlight/util.hpp>
#include <config.h>
#include "args.hpp"

namespace po = boost::program_options;

int parse_args(int argc, char **argv, struct args_struct *args)
{
    try
    {
        po::options_description desc("Allowed arguments");
        desc.add_options()
        ("help,h", "Produce help message")
        ("version,v", "Print version information")
        ("width,x", po::value<int>(), "Set window width in pixels")
        ("height,y", po::value<int>(), "Set window height in pixels");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc;
            args->quit_flag = true;
            return 1;
        }

        if (vm.count("version"))
        {
            std::cout << PROJECT_NAME " Ver. " PROJECT_VER << std::endl;
            lo_print_version();
            args->quit_flag = true;
            return 1;
        }

        if (vm.count("width"))
            args->width = vm["width"].as<int>();

        if (vm.count("height"))
            args->height = vm["height"].as<int>();
    }

    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 0;
    }

    catch (...)
    {
        std::cerr << "Error: unknown error" << std::endl;
        return 0;
    }

    return 1;
}
