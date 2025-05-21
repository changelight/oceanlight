#include "liboceanlight/lol_instance.hpp"
#include <GLFW/glfw3.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_glfw_callbacks.hpp>
#include <liboceanlight/lol_window.hpp>
#include <stdexcept>
#include <string>
#include <iostream>
#include <vulkan/vulkan.h>

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

		if (window_pointer == nullptr)
		{
			throw std::runtime_error("Could not create window");
		}

		glfwSetErrorCallback(lol_glfw_error_callback);
		glfwSetKeyCallback(window_pointer, lol_glfw_key_callback);
		glfwSetScrollCallback(window_pointer, lol_glfw_scroll_callback);
		glfwSetCursorPosCallback(window_pointer, lol_glfw_cursor_pos_callback);
		glfwSetWindowUserPointer(window_pointer, this);
		glfwSetFramebufferSizeCallback(window_pointer,
									   lol_glfw_framebuffer_size_callback);

		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(window_pointer,
							 GLFW_CURSOR,
							 GLFW_CURSOR_DISABLED);
			glfwSetInputMode(window_pointer, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
			std::cout << "RAW INPUT ENABLED\n";
		}
	}

	int window::should_close()
	{
		return glfwWindowShouldClose(window_pointer);
	}

	void window::create_surface(VkInstance& i)
	{
		VkResult rv = glfwCreateWindowSurface(i, window_pointer, nullptr, &surface);
		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create surface");
		}
	}
} /* namespace liboceanlight */
