#include <iostream>

void lo_print_error(int code, const char *description)
{
    std::cout << "Error " << code << ": " << description << std::endl;
}
