#include <iostream>
#include <liboceanlight/util.hpp>
 #include <config.h>

int lo_print_version()
{
    std::cout << PROJECT_NAME " Ver. " PROJECT_VER << std::endl;
    return 1;
}
