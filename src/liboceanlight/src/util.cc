#include <iostream>
#include <liboceanlight/util.hpp>
#include <config.h>

std::string lo::version_string()
{
    return PROJECT_NAME " Ver. " PROJECT_VER;
}
