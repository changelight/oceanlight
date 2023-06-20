#include <string>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_glfw_key_callback.hpp>
#include <liboceanlight/lol_glfw_err_callback.hpp>

namespace liboceanlight
{
	window::window(int w, int h) : width(w), height(h)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window_pointer = glfwCreateWindow(width,
										  height,
										  window_name.c_str(),
										  nullptr,
										  nullptr);

		if (window_pointer == NULL)
		{
			throw std::runtime_error("Could not create window");
		}
		
		glfwSetKeyCallback(window_pointer, lol_glfw_key_callback);
		glfwSetErrorCallback(lol_glfw_error_callback);
	}

	window::~window()
	{
		glfwDestroyWindow(window_pointer);
	}

	VkSurfaceKHR window::create_window_surface(VkInstance& instance)
	{
		VkSurfaceKHR surface {nullptr};
		VkResult rv = glfwCreateWindowSurface(instance,
											  window_pointer,
											  nullptr,
											  &surface);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Could not create window surface.");
		}

		return surface;
	}

	int window::should_close()
	{
		return glfwWindowShouldClose(window_pointer);
	}
}
