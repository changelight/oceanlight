#include <iostream>
#include <liboceanlight/error.hpp>

void lo::print_error(int code, const char *description)
{
    std::cerr << "Error " << code << ": " << description << "\n";
}