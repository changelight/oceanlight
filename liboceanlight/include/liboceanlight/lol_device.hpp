#ifndef LIBOCEANLIGHT_DEVICE_HPP_INCLUDED
#define LIBOCEANLIGHT_DEVICE_HPP_INCLUDED
#include <array>
#include <vulkan/vulkan_core.h>

namespace liboceanlight::engine
{
	using device_data = struct lol_device_data_struct
	{
		VkPhysicalDevice physical_device {VK_NULL_HANDLE};
		VkDevice logical_device {VK_NULL_HANDLE};
		static constexpr std::array dev_extensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		VkPhysicalDeviceProperties device_props {};
		VkPhysicalDeviceFeatures supported_device_features {};
		uint32_t graphics_queue_index {UINT32_MAX};
		VkQueue graphics_queue {nullptr};
	};
	void check_device_queue_support(VkSurfaceKHR&);
	void create_physical_device_new(VkInstance&);
} /* namespace liboceanlight::engine */
extern liboceanlight::engine::device_data dev_data;

#endif /* LIBOCEANLIGHT_DEVICE_HPP_INCLUDED */