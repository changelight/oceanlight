#include <iostream>
#include <liboceanlight/util.hpp>
#include <config.h>

int main(int argc, char **argv)
{
    std::cout << PROJECT_NAME " Ver. " PROJECT_VER << std::endl;
    lo_print_version();
    return EXIT_SUCCESS;
}