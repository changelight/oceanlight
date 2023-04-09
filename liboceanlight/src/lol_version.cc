#include <string>
#include <config.h>
#include <liboceanlight/lol_version.hpp>

std::string liboceanlight::version_string()
{
	return PROJECT_NAME " Ver. " PROJECT_VER;
}
