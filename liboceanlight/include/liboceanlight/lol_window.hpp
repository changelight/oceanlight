#ifndef LIBOCEANLIGHT_WINDOW_HPP_INCLUDED
#define LIBOCEANLIGHT_WINDOW_HPP_INCLUDED
#include <string>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace liboceanlight
{
	class window
	{
		int width, height;
		const std::string window_name {"Oceanlight"};

		public:
		GLFWwindow* window_pointer {nullptr};
		bool framebuffer_resized {false};
		window(int w, int h);
		~window();

		int should_close();
		VkSurfaceKHR create_window_surface(VkInstance&);
		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR&);
	};
}
#endif /* LIBOCEANLIGHT_WINDOW_HPP_INCLUDED */
