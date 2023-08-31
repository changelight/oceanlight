#ifndef LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <liboceanlight/lol_window.hpp>
#include <config.h>

namespace liboceanlight
{
	namespace engine
	{
		typedef struct lol_engine_data_struct
		{
			/* INSTANCE */
			VkDebugUtilsMessengerEXT dbg_messenger {nullptr};
			VkInstance vulkan_instance {nullptr};
#ifdef NDEBUG
			const bool validation_layer_enabled {false};
#else
			const bool validation_layer_enabled {true};
#endif /* NDEBUG */

			/* DEVICE */
			VkPhysicalDevice physical_device {VK_NULL_HANDLE};
			VkDevice logical_device {VK_NULL_HANDLE};
			static constexpr std::array dev_extensions {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			/* SURFACE */
			VkSurfaceKHR window_surface {nullptr};
			VkSurfaceCapabilitiesKHR capabilities {};
			VkSurfaceFormatKHR surface_format {};

			/* QUEUE */
			uint32_t graphics_queue_index {UINT32_MAX};
			VkQueue graphics_queue {nullptr};

			/* SWAPCHAIN */
			VkSwapchainKHR swap_chain {nullptr};
			VkExtent2D swap_extent {};
			VkPresentModeKHR present_mode;
			std::vector<VkImage> images;
			std::vector<VkImageView> image_views;
			std::vector<VkFramebuffer> frame_buffers;

			/* PIPELINE */
			VkPipelineLayout pipeline_layout {nullptr};
			VkRenderPass render_pass {nullptr};
			VkPipeline graphics_pipeline {nullptr};

			/* COMMAND */
			static constexpr int max_frames_in_flight {2};
			VkCommandPool command_pool {nullptr};
			std::array<VkCommandBuffer, max_frames_in_flight> command_buffers;

			/* DRAW */
			int current_frame {0};
			std::array<VkSemaphore, max_frames_in_flight> signal_sems;
			std::array<VkSemaphore, max_frames_in_flight> wait_sems;
			std::array<VkFence, max_frames_in_flight> in_flight_fences;
		} engine_data;

		void start(liboceanlight::window&, engine_data&);
		void run(liboceanlight::window&, engine_data&);
		void draw_frame(liboceanlight::window&, engine_data&);
		void record_cmd_buffer(engine_data&, VkCommandBuffer&, uint32_t);
		void recreate_swapchain(liboceanlight::window&, engine_data&);
	} /* NAMESPACE ENGINE */
} /* NAMESPACE_LIBOCEANLIGHT */
#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */
