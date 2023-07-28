#include <iostream>
#include <GLFW/glfw3.h>
#include <liboceanlight/lol_window.hpp>

void lol_glfw_error_callback(int error_code, const char* description)
{
	std::cerr << "Error " << error_code << ": " << description << "\n";
}

void lol_glfw_key_callback(GLFWwindow* window,
						   int key,
						   int scancode,
						   int action,
						   int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void lol_glfw_framebuffer_size_callback(GLFWwindow* window_pointer,
										int width,
										int height)
{
	auto window = reinterpret_cast<liboceanlight::window*>(
		glfwGetWindowUserPointer(window_pointer));
	window->framebuffer_resized = true;
}
