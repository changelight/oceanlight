#ifndef LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#include "vulkan/vulkan_core.h"
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_glfw_key_callback.hpp>
#include <liboceanlight/lol_glfw_err_callback.hpp>
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
		VkInstance vulkan_instance {nullptr};
		VkDevice logical_device {nullptr};
		VkPhysicalDevice physical_device {nullptr};
		VkQueue graphics_queue {nullptr};
		VkQueue present_queue {nullptr};
		VkSurfaceKHR window_surface {nullptr};
		const bool validation_layers_enabled {true};
		VkDebugUtilsMessengerEXT debug_utils_messenger {nullptr};
		queue_family_indices_struct indices {};
		const std::vector<const char*> device_extensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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
			if (window_surface)
			{
				vkDestroySurfaceKHR(vulkan_instance, window_surface, nullptr);
			}

			if (debug_utils_messenger)
			{
				DestroyDebugUtilsMessengerEXT(vulkan_instance,
											  debug_utils_messenger,
											  nullptr);
			}

			vkDestroyDevice(logical_device, nullptr);
			vkDestroyInstance(vulkan_instance, nullptr);
			glfwTerminate();
		}

		void init(liboceanlight::lol_window&);
		VkInstance create_vulkan_instance();
		void run(liboceanlight::lol_window&);
		VkPhysicalDevice pick_physical_device();
		void find_queue_families();
		struct swap_chain_support_details get_swap_chain_support_details();
	};
} /* namespace liboceanlight */

VkPhysicalDevice pick_physical_device(VkInstance&,
									  queue_family_indices_struct&);

VkDevice create_logical_device(VkPhysicalDevice&,
							   queue_family_indices_struct&,
							   const std::vector<const char*>&);

bool check_vldn_layer_support(
	const std::vector<const char*>& validation_layers);

void enable_vldn_layers(VkInstanceCreateInfo&, std::vector<const char*>&);
void enable_dbg_utils_msngr(std::vector<const char*>&,
							VkDebugUtilsMessengerCreateInfoEXT&,
							VkInstanceCreateInfo&);

bool device_is_suitable(queue_family_indices_struct&,
						VkPhysicalDevice&,
						VkSurfaceKHR&,
						const std::vector<const char*>&);

uint32_t rate_device_suitability(const VkPhysicalDevice&);
VkApplicationInfo populate_instance_app_info(void);
VkInstanceCreateInfo populate_instance_create_info(VkApplicationInfo&);
VkDeviceQueueCreateInfo populate_queue_create_info(uint32_t&);

VkDeviceCreateInfo populate_device_create_info(
	VkPhysicalDeviceFeatures&,
	std::vector<VkDeviceQueueCreateInfo>&,
	const std::vector<const char*>&);

std::vector<const char*> get_required_instance_extensions(void);
#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */