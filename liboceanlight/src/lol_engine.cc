#include <iostream>
#include <stdint.h>
#include <vector>
#include <map>
#include <optional>
#include <set>
#include <cstring>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_window.hpp>
#include <config.h>

void liboceanlight::engine::run(liboceanlight::lol_window& window)
{
	while (!window.should_close())
	{
		// glfwSwapBuffers(window);
		glfwWaitEvents();
	}
}

void liboceanlight::engine::init(liboceanlight::lol_window& window)
{
	vulkan_instance = create_vulkan_instance();
	window_surface = window.create_window_surface(vulkan_instance);
	physical_device = pick_physical_device();

	find_queue_families();

	if (!device_is_suitable(indices, physical_device, device_extensions))
	{
		throw std::runtime_error("Device lacks required queue family.");
	}

	logical_device = create_logical_device(physical_device,
										   indices,
										   device_extensions);

	vkGetDeviceQueue(logical_device,
					 indices.graphics_queue_family.value(),
					 0,
					 &graphics_queue);

	if (!graphics_queue)
	{
		throw std::runtime_error("Could not get device queue handle.");
	}

	vkGetDeviceQueue(logical_device,
					 indices.presentation_queue_family.value(),
					 0,
					 &present_queue);

	if (!present_queue)
	{
		throw std::runtime_error("Could not get presentation queue handle.");
	}
}

VkInstance liboceanlight::engine::create_vulkan_instance()
{
	VkApplicationInfo instance_app_info;
	instance_app_info = populate_instance_app_info();

	VkInstanceCreateInfo instance_create_info;
	instance_create_info = populate_instance_create_info(instance_app_info);

	std::vector<const char*> extensions = get_required_instance_extensions();
	std::vector<const char*> vldn_layers {"VK_LAYER_KHRONOS_validation"};

	VkDebugUtilsMessengerCreateInfoEXT dbg_utils_msngr_create_info {};
	if (validation_layers_enabled && check_vldn_layer_support(vldn_layers))
	{
		enable_vldn_layers(instance_create_info, vldn_layers);
		enable_dbg_utils_msngr(extensions,
							   dbg_utils_msngr_create_info,
							   instance_create_info);
	}

	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_create_info.ppEnabledExtensionNames = extensions.data();

	uint32_t extension_count {0};
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> vulkan_extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr,
										   &extension_count,
										   vulkan_extensions.data());

	VkInstance instance {nullptr};
	VkResult rv = vkCreateInstance(&instance_create_info, nullptr, &instance);

	if (rv != VK_SUCCESS || !instance)
	{
		throw std::runtime_error("Failed to create Vulkan instance.");
	}

	if (validation_layers_enabled)
	{
		rv = CreateDebugUtilsMessengerEXT(instance,
										  &dbg_utils_msngr_create_info,
										  nullptr,
										  &debug_utils_messenger);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger.");
		}
	}

	return instance;
}

VkPhysicalDevice liboceanlight::engine::pick_physical_device()
{
	uint32_t device_count {0};
	VkPhysicalDevice physical_device {VK_NULL_HANDLE};
	vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);

	if (device_count == 0)
	{
		throw std::runtime_error("Failed to find supported device");
	}

	std::vector<VkPhysicalDevice> physical_devices(device_count);
	vkEnumeratePhysicalDevices(vulkan_instance,
							   &device_count,
							   physical_devices.data());

	std::multimap<uint32_t, VkPhysicalDevice> device_candidates;
	for (const auto& device : physical_devices)
	{
		uint32_t score = rate_device_suitability(device);
		device_candidates.insert(std::make_pair(score, device));

		/* Multimap is sorted, so rbegin()->first contains highest score */
		if (device_candidates.rbegin()->first > 0)
		{
			physical_device = device_candidates.rbegin()->second;
		}
		else
		{
			throw std::runtime_error("Failed to find suitable device.");
		}
	}

	if (physical_device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Found devices, but none suitable.");
	}

	return physical_device;
}

VkDevice create_logical_device(
	VkPhysicalDevice& physical_device,
	queue_family_indices_struct& indices,
	const std::vector<const char*>& device_extensions)
{
	VkDevice logical_device {nullptr};

	std::set<uint32_t> unique_queue_families {
		indices.graphics_queue_family.value(),
		indices.presentation_queue_family.value()};
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	for (uint32_t queue_family : unique_queue_families)
	{
		queue_create_infos.push_back(populate_queue_create_info(queue_family));
	}

	VkPhysicalDeviceFeatures features {};
	VkDeviceCreateInfo device_create_info;
	device_create_info = populate_device_create_info(features,
													 queue_create_infos,
													 device_extensions);

	VkResult rv;
	rv = vkCreateDevice(physical_device,
						&device_create_info,
						nullptr,
						&logical_device);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Logical device creation failed");
	}

	return logical_device;
}

uint32_t rate_device_suitability(const VkPhysicalDevice& physical_device)
{
	uint32_t score {0};

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(physical_device, &device_features);

	if (!device_features.geometryShader)
	{
		return 0;
	}

	if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 100;
	}

	score += device_properties.limits.maxImageDimension2D;

	return score;
}

void liboceanlight::engine::find_queue_families()
{
	VkBool32 present_support {false};
	uint32_t queue_family_count {0};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
											 &queue_family_count,
											 nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
											 &queue_family_count,
											 queue_families.data());

	VkResult rv;
	int i {0};
	for (const auto& queue_family : queue_families)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics_queue_family = i;
		}

		rv = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
												  i,
												  window_surface,
												  &present_support);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Could not determine surface support");
		}

		if (present_support)
		{
			indices.presentation_queue_family = i;
		}

		++i;
	}
}

bool check_device_extension_support(
	VkPhysicalDevice& device,
	const std::vector<const char*>& device_extensions)
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device,
										 nullptr,
										 &extension_count,
										 nullptr);

	std::vector<VkExtensionProperties> extension_properties(extension_count);
	vkEnumerateDeviceExtensionProperties(device,
										 nullptr,
										 &extension_count,
										 extension_properties.data());

	std::set<std::string> required_extensions(device_extensions.begin(),
											  device_extensions.end());

	for (const auto& extension : extension_properties)
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}

bool device_is_suitable(queue_family_indices_struct& indices,
						VkPhysicalDevice& device,
						const std::vector<const char*>& device_extensions)
{
	bool extensions_supported = check_device_extension_support(
		device,
		device_extensions);

	return indices.graphics_queue_family.has_value() &&
		   indices.presentation_queue_family.has_value() &&
		   extensions_supported;
}

void enable_dbg_utils_msngr(
	std::vector<const char*>& extensions,
	VkDebugUtilsMessengerCreateInfoEXT& dbg_utils_msngr_create_info,
	VkInstanceCreateInfo& instance_create_info)
{
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	dbg_utils_msngr_create_info = populate_dbg_utils_msngr_create_info();
	instance_create_info.pNext =
		(VkDebugUtilsMessengerCreateInfoEXT*)&dbg_utils_msngr_create_info;
}

void enable_vldn_layers(VkInstanceCreateInfo& c_info,
						std::vector<const char*>& vldn_layers)
{
	c_info.enabledLayerCount = static_cast<uint32_t>(vldn_layers.size());
	c_info.ppEnabledLayerNames = vldn_layers.data();
}

bool check_vldn_layer_support(const std::vector<const char*>& vldn_layers)
{
	uint32_t count {0};
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	std::vector<VkLayerProperties> layer_properties(count);
	vkEnumerateInstanceLayerProperties(&count, layer_properties.data());

	bool layer_found;
	for (const char* layer : vldn_layers)
	{
		layer_found = false;

		for (const auto& i : layer_properties)
		{
			if (strcmp(layer, i.layerName) == 0)
			{
				layer_found = true;
				std::cout << "Found Validation Layer: \n" << layer << "\n";
			}
		}

		if (!layer_found)
		{
			std::cerr << "LAYER NOT FOUND: " << layer << "\n";
			throw std::runtime_error("Did not find validation layer.");
		}
	}
	return true;
}

std::vector<const char*> get_required_instance_extensions()
{
	uint32_t count {0};
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions_vec(extensions, extensions + count);
	return extensions_vec;
}

VkInstanceCreateInfo populate_instance_create_info(VkApplicationInfo& app_info)
{
	VkInstanceCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.pNext = nullptr;
	return create_info;
}

VkApplicationInfo populate_instance_app_info()
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

	return app_info;
}

VkInstanceCreateInfo populate_instance_create_info(VkApplicationInfo* app_info)
{
	VkInstanceCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = app_info;
	create_info.pNext = nullptr;
	return create_info;
}

VkDeviceQueueCreateInfo populate_queue_create_info(uint32_t& queue_family)
{
	VkDeviceQueueCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	create_info.queueFamilyIndex = queue_family;
	create_info.queueCount = 1;
	float queue_priority = 1.0f;
	create_info.pQueuePriorities = &queue_priority;
	return create_info;
}

VkDeviceCreateInfo populate_device_create_info(
	VkPhysicalDeviceFeatures& features,
	std::vector<VkDeviceQueueCreateInfo>& queue_create_infos,
	const std::vector<const char*>& device_extensions)
{
	VkDeviceCreateInfo create_info {};
	create_info.queueCreateInfoCount = static_cast<uint32_t>(
		queue_create_infos.size());
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pEnabledFeatures = &features;
	create_info.enabledExtensionCount = static_cast<uint32_t>(
		device_extensions.size());
	create_info.ppEnabledExtensionNames = device_extensions.data();

	return create_info;
}
