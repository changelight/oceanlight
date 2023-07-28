#ifndef LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#include "vulkan/vulkan_core.h"
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_glfw_callbacks.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <config.h>
#include <liboceanlight/lol_window.hpp>

struct queue_family_indices_struct
{
	std::optional<uint32_t> graphics_queue_family;
	std::optional<uint32_t> presentation_queue_family;

	bool is_complete()
	{
		return graphics_queue_family.has_value() &&
			   presentation_queue_family.has_value();
	}
};

struct swap_chain_support_details
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

namespace liboceanlight
{
	class engine
	{
		VkDebugUtilsMessengerEXT debug_utils_messenger {nullptr};
		VkInstance vulkan_instance {nullptr};
		VkDevice logical_device {nullptr};
		VkPhysicalDevice physical_device {nullptr};
		VkQueue graphics_queue {nullptr}, present_queue {nullptr};
		VkSurfaceKHR window_surface {nullptr};
		VkSwapchainKHR swap_chain {nullptr};
		queue_family_indices_struct queue_family_indices {};
		const std::vector<const char*> device_extensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		swap_chain_support_details swap_details {};
		VkExtent2D swap_chain_extent {};
		VkFormat swap_chain_image_format;
		std::vector<VkImage> swap_chain_images;
		std::vector<VkImageView> swap_chain_image_views;
		VkRenderPass render_pass {nullptr};
		VkPipelineLayout pipeline_layout {nullptr};
		VkPipeline graphics_pipeline {nullptr};
		std::vector<VkFramebuffer> swap_chain_frame_buffers;
		VkCommandPool command_pool {nullptr};
		std::vector<VkCommandBuffer> command_buffers;
		std::vector<VkSemaphore> image_available_semaphores;
		std::vector<VkSemaphore> rendering_finished_semaphores;
		std::vector<VkFence> in_flight_fences;
		const unsigned short int max_frames_in_flight {2};
		unsigned short int current_frame {0};

#ifdef NDEBUG
		const bool validation_layers_enabled {false};
#else
		const bool validation_layers_enabled {true};
#endif

		public:
		engine()
		{
			glfwSetErrorCallback(lol_glfw_error_callback);

			int rv = glfwInit();
			if (!rv)
			{
				throw std::runtime_error("Failed to initialize glfw");
			}
		}

		~engine()
		{
			cleanup_swap_chain();

			for (auto i {0}; i < max_frames_in_flight; ++i)
			{
				vkDestroySemaphore(logical_device,
								   image_available_semaphores[i],
								   nullptr);

				vkDestroySemaphore(logical_device,
								   rendering_finished_semaphores[i],
								   nullptr);

				vkDestroyFence(logical_device, in_flight_fences[i], nullptr);
			}

			vkDestroyCommandPool(logical_device, command_pool, nullptr);

			vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
			vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
			vkDestroyRenderPass(logical_device, render_pass, nullptr);

			if (window_surface)
			{
				vkDestroySurfaceKHR(vulkan_instance, window_surface, nullptr);
			}

			vkDestroyDevice(logical_device, nullptr);

			if (debug_utils_messenger)
			{
				DestroyDebugUtilsMessengerEXT(vulkan_instance,
											  debug_utils_messenger,
											  nullptr);
			}

			vkDestroyInstance(vulkan_instance, nullptr);
			glfwTerminate();
		}

		void cleanup_swap_chain()
		{
			for (auto framebuffer : swap_chain_frame_buffers)
			{
				vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
			}

			for (auto image_view : swap_chain_image_views)
			{
				vkDestroyImageView(logical_device, image_view, nullptr);
			}

			vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
		}

		void init(liboceanlight::window&);
		VkInstance create_vulkan_instance();
		void run(liboceanlight::window&);
		VkPhysicalDevice pick_physical_device();
		queue_family_indices_struct find_queue_families();
		bool device_is_suitable(queue_family_indices_struct&,
								VkPhysicalDevice&,
								VkSurfaceKHR&,
								const std::vector<const char*>&);
		VkSwapchainKHR create_swap_chain(liboceanlight::window&);
		void recreate_swap_chain(liboceanlight::window&);
		std::vector<VkImageView> create_image_views();
		void create_graphics_pipeline();
		VkShaderModule create_shader_module(const std::vector<char>&);
		void create_render_pass();
		void create_framebuffers();
		void create_command_pool();
		void create_command_buffers();
		void record_command_buffer(VkCommandBuffer&, uint32_t);
		void draw_frame(liboceanlight::window&);
		void create_sync_objects();
	};
} /* namespace liboceanlight */

VkPhysicalDevice pick_physical_device(VkInstance&,
									  queue_family_indices_struct&);

VkDevice create_logical_device(VkPhysicalDevice&,
							   queue_family_indices_struct&,
							   const std::vector<const char*>&);

bool check_layer_support(const std::vector<const char*>&);
bool check_extension_support(const std::vector<const char*>&);
void setup_instance_layers(VkInstanceCreateInfo&, std::vector<const char*>&);
void setup_instance_extensions(VkInstanceCreateInfo&,
							   std::vector<const char*>&);
void setup_dbg_utils_msngr(std::vector<const char*>&,
						   VkDebugUtilsMessengerCreateInfoEXT&,
						   VkInstanceCreateInfo&);

uint32_t rate_device_suitability(const VkPhysicalDevice&);
VkApplicationInfo populate_instance_app_info(void);
VkInstanceCreateInfo populate_instance_create_info(VkApplicationInfo&);
VkDeviceQueueCreateInfo populate_queue_create_info(uint32_t&);

VkDeviceCreateInfo populate_device_create_info(
	VkPhysicalDeviceFeatures&,
	std::vector<VkDeviceQueueCreateInfo>&,
	const std::vector<const char*>&);

std::vector<const char*> get_required_instance_extensions(void);
struct swap_chain_support_details get_swap_chain_support_details(
	VkPhysicalDevice&,
	VkSurfaceKHR&);
VkSurfaceFormatKHR choose_swap_surface_format(
	const std::vector<VkSurfaceFormatKHR>&);
#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */
