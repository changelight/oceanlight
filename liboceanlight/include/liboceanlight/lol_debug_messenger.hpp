#ifndef LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED
#define LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED
#include <vector>
#include <vulkan/vulkan.h>

namespace liboceanlight::engine
{
	using dbg_messenger_data = struct lol_debug_messenger_data_struct
	{
		VkDebugUtilsMessengerEXT dbg_messenger {nullptr};
	};
	void create_dbg_messenger(VkDebugUtilsMessengerCreateInfoEXT&);
	bool check_vldn_layer_support(const std::vector<const char*>&);
	void set_dbg_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT&);

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance,
		const VkDebugUtilsMessengerCreateInfoEXT*,
		const VkAllocationCallbacks*,
		VkDebugUtilsMessengerEXT*);

	void DestroyDebugUtilsMessengerEXT(VkInstance,
									   VkDebugUtilsMessengerEXT,
									   const VkAllocationCallbacks*);

	VKAPI_ATTR VkBool32 VKAPI_CALL dbg_messenger_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		void* user_data);
} /* namespace liboceanlight::engine */
#endif /* LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED */
