#include <vector>
#include <stdexcept>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_instance.hpp>

using namespace liboceanlight::engine;
device_data dev_data;

VkPhysicalDevice select_physical_device(std::vector<VkPhysicalDevice>& devs)
{
	if (devs.empty())
	{
		return VK_NULL_HANDLE;
	}

	VkPhysicalDeviceProperties props {};
	VkPhysicalDeviceFeatures features {};
	std::vector<unsigned long int> scores(devs.size());
	unsigned long int resulting_dev_index {0}, highest_score {0};
	constexpr unsigned long int bonus_discrete {64}, bonus_integrated {8};

	for (size_t i {0}; i < devs.size(); ++i)
	{
		vkGetPhysicalDeviceProperties(devs[i], &props);
		vkGetPhysicalDeviceFeatures(devs[i], &features);

		if (!features.geometryShader)
		{
			scores[i] = 0;
			continue;
		}

		if (!features.samplerAnisotropy)
		{
			scores[i] = 0;
			continue;
		}

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			scores[i] += bonus_discrete;
		}
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			scores[i] += bonus_integrated;
		}

		scores[i] += props.limits.maxImageDimension2D;
		if (scores[i] > highest_score)
		{
			highest_score = scores[i];
			resulting_dev_index = i;
		}
	}

	if (highest_score == 0)
	{
		throw std::runtime_error("Could not find suitable graphics device");
	}

	vkGetPhysicalDeviceProperties(devs[resulting_dev_index],
								  &dev_data.device_props);
	vkGetPhysicalDeviceFeatures(devs[resulting_dev_index],
								&dev_data.supported_device_features);

	return devs[resulting_dev_index];
}

void check_device_extension_support()
{
	uint32_t count {0};
	auto rv = vkEnumerateDeviceExtensionProperties(dev_data.physical_device,
												   nullptr,
												   &count,
												   nullptr);
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get device extensions (count)");
	}

	std::vector<VkExtensionProperties> ext_props(count);
	vkEnumerateDeviceExtensionProperties(dev_data.physical_device,
										 nullptr,
										 &count,
										 ext_props.data());
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get device extensions (props)");
	}

	unsigned int supported_ext_count {0};
	for (auto required_ext : dev_data.dev_extensions)
	{
		for (auto supported_ext : ext_props)
		{
			if (strcmp(required_ext, supported_ext.extensionName) == 0)
			{
				++supported_ext_count;
			}
		}
	}

	if (supported_ext_count != dev_data.dev_extensions.size())
	{
		throw std::runtime_error("Not all device extensions supported");
	}
}

void liboceanlight::engine::check_device_queue_support(VkSurfaceKHR& surface)
{
	uint32_t count {0};
	vkGetPhysicalDeviceQueueFamilyProperties(dev_data.physical_device,
											 &count,
											 nullptr);

	std::vector<VkQueueFamilyProperties> queue_fam_props(count);
	vkGetPhysicalDeviceQueueFamilyProperties(dev_data.physical_device,
											 &count,
											 queue_fam_props.data());

	std::vector<VkBool32> supports_graphics(count);
	std::vector<VkBool32> supports_presentation(count);
	for (size_t i {0}; i < queue_fam_props.size(); ++i)
	{
		if (queue_fam_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			supports_graphics[i] = true;
		}

		VkResult rv = vkGetPhysicalDeviceSurfaceSupportKHR(
			dev_data.physical_device,
			i,
			surface,
			&supports_presentation[i]);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to get present support");
		}

		if (supports_graphics[i] && supports_presentation[i])
		{
			dev_data.graphics_queue_index = i;
			break;
		}
	}

	if (dev_data.graphics_queue_index == UINT32_MAX)
	{
		throw std::runtime_error("Failed to find appropriate queue");
	}
}

void liboceanlight::engine::create_physical_device_new(VkInstance& instance)
{
	uint32_t count {0};
	VkResult rv = vkEnumeratePhysicalDevices(instance, &count, nullptr);
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to enumerate physical device count");
	}

	std::vector<VkPhysicalDevice> devices(count);
	rv = vkEnumeratePhysicalDevices(instance, &count, devices.data());
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to enumerate physical devices");
	}

	dev_data.physical_device = select_physical_device(devices);
	if (!dev_data.physical_device)
	{
		throw std::runtime_error("Failed to select physical device");
	}

	check_device_extension_support();
}
