#ifndef LIBOCEANLIGHT_WINDOW_HPP_INCLUDED
#define LIBOCEANLIGHT_WINDOW_HPP_INCLUDED
#include <GLFW/glfw3.h>
#include <config.h>
#include <string>
#include <utility>

namespace liboceanlight
{
	class window
	{
		int width, height;
		std::string window_name {PROJECT_NAME};

	  public:
		GLFWwindow* window_pointer {nullptr};
		bool framebuffer_resized {false};

		window(int w, int h);
		window(const window& w) = delete;
		window& operator=(const window&) = delete;
		window(window&& old_window) noexcept :
			width(old_window.width),
			height(old_window.height),
			window_name(std::move(old_window.window_name)),
			window_pointer(std::exchange(old_window.window_pointer, nullptr)),
			framebuffer_resized(old_window.framebuffer_resized)
		{
		}

		window& operator=(window&& old_window) noexcept
		{
			if (window_pointer)
			{
				glfwDestroyWindow(window_pointer);
			}

			width = old_window.width;
			height = old_window.height;
			window_name = std::move(old_window.window_name);
			window_pointer = std::exchange(old_window.window_pointer, nullptr);
			framebuffer_resized = old_window.framebuffer_resized;
			window_pointer = std::exchange(old_window.window_pointer, nullptr);
			return *this;
		}

		~window() noexcept
		{
			if (window_pointer)
			{
				glfwDestroyWindow(window_pointer);
			}
		}

		int should_close();
	};
} /* namespace liboceanlight */
#endif /* LIBOCEANLIGHT_WINDOW_HPP_INCLUDED */
