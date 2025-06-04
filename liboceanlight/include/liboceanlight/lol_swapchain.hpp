#ifndef LIBOCEANLIGHT_SWAPCHAIN_HPP_INCLUDED
#define LIBOCEANLIGHT_SWAPCHAIN_HPP_INCLUDED
#include <vector>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_window.hpp>

namespace liboceanlight::swapchain
{
	using swapchain_data = struct lol_swapchain_data_struct
	{
		VkSwapchainKHR swap_chain {nullptr};
		VkExtent2D swap_extent {};
		VkPresentModeKHR present_mode;
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;
		std::vector<VkFramebuffer> frame_buffers;
	};
	void get_swapchain_details_new(liboceanlight::window&);
} /* namespace liboceanlight::engine */
extern liboceanlight::swapchain::swapchain_data swap_data;
#endif /* LIBOCEANLIGHT_SWAPCHAIN_HPP_INCLUDED */
