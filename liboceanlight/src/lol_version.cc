#include <config.h>
#include <liboceanlight/lol_version.hpp>
#include <string>

std::string liboceanlight::version_string()
{
	return PROJECT_NAME " Ver. " PROJECT_VER;
}
