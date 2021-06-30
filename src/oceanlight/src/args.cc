#include <iostream>
#include <boost/program_options.hpp>
#include <liboceanlight/util.hpp>
#include <config.h>

namespace po = boost::program_options;

int parse_args(int argc, char **argv)
{
    po::options_description desc("Allowed arguments");
    desc.add_options()
        ("help,h", "Produce help message")
        ("version,v", "Print version information");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
        std::cout << desc;

    if (vm.count("version"))
    {
        std::cout << PROJECT_NAME " Ver. " PROJECT_VER << std::endl;
        lo_print_version();
    }

    return 1;
}