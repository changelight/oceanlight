#include <iostream>
#include <liboceanlight/util.hpp>
#include <config.h>

std::string lo::version_string()
{
    std::string version_string(PROJECT_NAME " Ver. " PROJECT_VER "\n");
    return version_string;
}
