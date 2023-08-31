#include <iostream>
#include <vulkan/vulkan.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <config.h>

VKAPI_ATTR VkBool32 VKAPI_CALL liboceanlight::engine::dbg_messenger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data)
{
	std::cout << "⦕ VLDN ⦖ " << callback_data->pMessage << "\n";

	return VK_FALSE;
}

void liboceanlight::engine::set_dbg_messenger_create_info(
	VkDebugUtilsMessengerCreateInfoEXT& c_info)
{
	c_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	c_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	c_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
						 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	c_info.pfnUserCallback = dbg_messenger_callback;
	c_info.pUserData = nullptr; // Optional
}

VkResult liboceanlight::engine::CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance,
		"vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void liboceanlight::engine::DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance,
		"vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}
