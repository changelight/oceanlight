#include <string>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_glfw_callbacks.hpp>

namespace liboceanlight
{
	window::window(int w, int h) : width(w), height(h)
	{
		glfwSetErrorCallback(lol_glfw_error_callback);

		int rv = glfwInit();
		if (rv != GLFW_TRUE)
		{
			throw std::runtime_error("Failed to initialize glfw");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window_pointer = glfwCreateWindow(width,
										  height,
										  window_name.c_str(),
										  nullptr,
										  nullptr);

		if (window_pointer == NULL)
		{
			throw std::runtime_error("Could not create window");
		}

		glfwSetErrorCallback(lol_glfw_error_callback);
		glfwSetKeyCallback(window_pointer, lol_glfw_key_callback);
		glfwSetWindowUserPointer(window_pointer, this);
		glfwSetFramebufferSizeCallback(window_pointer,
									   lol_glfw_framebuffer_size_callback);
	}

	window::window(const window& w) : window(w.width, w.height) {}
	window::~window()
	{
		glfwDestroyWindow(window_pointer);
	}

	int window::should_close()
	{
		return glfwWindowShouldClose(window_pointer);
	}
}
