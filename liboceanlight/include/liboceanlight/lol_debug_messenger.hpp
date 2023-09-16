#ifndef LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED
#define LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED
#include <vulkan/vulkan.h>
#include <vector>

namespace liboceanlight
{
	namespace engine
	{
		typedef struct lol_debug_messenger_data_struct
		{
			VkDebugUtilsMessengerEXT dbg_messenger {nullptr};
		} dbg_messenger_data;
		void create_dbg_messenger(VkDebugUtilsMessengerCreateInfoEXT&);
		bool check_vldn_layer_support(const std::vector<const char*>&);
		void set_dbg_messenger_create_info(
			VkDebugUtilsMessengerCreateInfoEXT&);

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
	} /* namespace engine */
} /* namespace liboceanlight */
#endif /* LIBOCEANLIGHT_DEBUG_MESSENGER_HPP_INCLUDED */
