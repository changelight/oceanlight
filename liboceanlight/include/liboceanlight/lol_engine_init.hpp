#ifndef LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_swapchain.hpp>

namespace liboceanlight::engine_init
{
	using engine_init_data = struct lol_engine_init_data
	{
		VkCommandPool command_pool {nullptr};
		VkImage depth_img {nullptr};
		VkDeviceMemory depth_img_mem {nullptr};
		VkImageView depth_img_view {nullptr};
		VkFormat depth_fmt {VK_FORMAT_D24_UNORM_S8_UINT};
		std::vector<VkFramebuffer> frame_buffers;
	};
	int init(window&);
	void create_cmd_pool(void);
	uint32_t find_mem_type(uint32_t, VkMemoryPropertyFlags);
	void create_depth_image();
	VkCommandBuffer begin_single_time_cmds(void);
	void end_single_time_cmds(VkCommandBuffer);
	void transition_img_layout(VkImage,
							   VkFormat,
							   VkImageLayout,
							   VkImageLayout);
	void create_framebuffers(VkDevice,
							 VkRenderPass,
							 swapchain::swapchain_data&);
	void create_sync_objects(VkDevice,
							 const int,
							 VkSemaphore*,
							 VkSemaphore*,
							 VkFence*);
} /* namespace liboceanlight::engine_init */
extern liboceanlight::engine_init::engine_init_data init_data;

#endif /* LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED */
