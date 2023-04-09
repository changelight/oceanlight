#ifndef LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED
#define LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED
#include <iostream>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace liboceanlight
{
	std::string version_string(void);
}

bool check_vldn_layer_support(const std::vector<const char*>&);
VkApplicationInfo populate_vulkan_application_info();
VkInstanceCreateInfo populate_vulkan_create_info(VkApplicationInfo*);
VkDebugUtilsMessengerCreateInfoEXT populate_dbg_utils_msngr_create_info();

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance,
	const VkDebugUtilsMessengerCreateInfoEXT*,
	const VkAllocationCallbacks*,
	VkDebugUtilsMessengerEXT*);

void DestroyDebugUtilsMessengerEXT(VkInstance,
								   VkDebugUtilsMessengerEXT,
								   const VkAllocationCallbacks*);

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data);

#endif /* LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED */
