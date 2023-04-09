#include <iostream>

void lol_glfw_error_callback(int error_code, const char* description)
{
	std::cerr << "Error " << error_code << ": " << description << "\n";
}
