#ifndef LIBOCEANLIGHT_WINDOW_HPP_INCLUDED
#define LIBOCEANLIGHT_WINDOW_HPP_INCLUDED
#include <string>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace liboceanlight
{
	class lol_window
	{
		int width, height;
		const std::string window_name {"Oceanlight"};
		GLFWwindow* window_pointer {nullptr};

		public:
		lol_window(int w, int h);
		~lol_window();

		VkSurfaceKHR create_window_surface(VkInstance&);
		int should_close();
	};
}
#endif /* LIBOCEANLIGHT_WINDOW_HPP_INCLUDED */
