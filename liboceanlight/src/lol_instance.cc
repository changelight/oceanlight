#include <config.h>
#include <vector>
#include <stdexcept>
#include <span>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_instance.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>

using namespace liboceanlight::engine;
instance_data inst_data;

std::vector<const char*> get_required_extensions()
{
	uint32_t count {0};
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	if (!extensions | !count)
	{
		throw std::runtime_error("Failed to get required extensions");
	}

	std::span span {extensions, count};
	return {span.begin(), span.end()};
}

void set_dbg_msngr_create_info(VkDebugUtilsMessengerCreateInfoEXT& c_info)
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

void check_inst_ext_support(const std::vector<const char*>& required)
{
	uint32_t count {0};
	auto rv = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get instance extensions (count)");
	}

	std::vector<VkExtensionProperties> extension_properties(count);
	vkEnumerateInstanceExtensionProperties(nullptr,
										   &count,
										   extension_properties.data());
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get instance extensions (props)");
	}

	unsigned int valid_extension_count {0};
	for (auto required_ext : required)
	{
		for (int i {0}; i < extension_properties.size(); ++i)
		{
			if (strcmp(required_ext, extension_properties[i].extensionName) ==
				0)
			{
				++valid_extension_count;
			}
		}
	}

	if (valid_extension_count != required.size())
	{
		throw std::runtime_error("Not all extensions supported");
	}
}

void check_layer_support(const std::vector<const char*>& required)
{
	uint32_t count {0};
	auto rv = vkEnumerateInstanceLayerProperties(&count, nullptr);
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to enumerate layers (count)");
	}

	std::vector<VkLayerProperties> layer_properties(count);
	vkEnumerateInstanceLayerProperties(&count, layer_properties.data());
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to enumerate layers (props)");
	}

	unsigned int supported_layer_count {0};
	for (auto required_layer : required)
	{
		for (int i {0}; i < layer_properties.size(); ++i)
		{
			if (strcmp(required_layer, layer_properties[i].layerName) == 0)
			{
				++supported_layer_count;
			}
		}
	}

	if (supported_layer_count != required.size())
	{
		throw std::runtime_error("Not all layers supported");
	}
}

int create_instance_new()
{
	VkApplicationInfo app_info {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = PROJECT_NAME;
	app_info.pEngineName = PROJECT_NAME;
	app_info.apiVersion = VK_API_VERSION_1_3;
	app_info.pNext = nullptr;

	app_info.applicationVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR,
												  PROJECT_VER_MINOR,
												  PROJECT_VER_PATCH);

	app_info.engineVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR,
											 PROJECT_VER_MINOR,
											 PROJECT_VER_PATCH);

	VkInstanceCreateInfo inst_info {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.pApplicationInfo = &app_info;

	std::vector<const char*> req_exts = get_required_extensions();
	std::vector<const char*> req_layers {};
	VkDebugUtilsMessengerCreateInfoEXT dbg_info {};
	if (inst_data.validation_layer_enabled)
	{
		req_layers.push_back("VK_LAYER_KHRONOS_validation");
		req_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		set_dbg_messenger_create_info(dbg_info);
		inst_info.pNext = &dbg_info;
	}

	if (!req_exts.empty())
	{
		check_inst_ext_support(req_exts);
		inst_info.enabledExtensionCount = static_cast<uint32_t>(
			req_exts.size());
		inst_info.ppEnabledExtensionNames = req_exts.data();
	}

	if (!req_layers.empty())
	{
		check_layer_support(req_layers);
		inst_info.enabledLayerCount = static_cast<uint32_t>(req_layers.size());
		inst_info.ppEnabledLayerNames = req_layers.data();
	}

	VkResult rv = vkCreateInstance(&inst_info,
								   nullptr,
								   &inst_data.vulkan_instance);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vulkan instance");
	}

	if (inst_data.validation_layer_enabled)
	{
		rv = CreateDebugUtilsMessengerEXT(inst_data.vulkan_instance,
										  &dbg_info,
										  nullptr,
										  &inst_data.dbg_messenger);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create debug messenger");
		}
	}

	return 1;
}
