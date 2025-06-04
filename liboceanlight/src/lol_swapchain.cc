#include <stdexcept>
#include <algorithm>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_swapchain.hpp>
#include <liboceanlight/lol_device.hpp>

liboceanlight::swapchain::swapchain_data swap_data;

void liboceanlight::swapchain::get_swapchain_details_new(
	liboceanlight::window& window)
{
	VkResult rv = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		dev_data.physical_device,
		window.surface,
		&window.surface_capabilities);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get surface capabilities");
	}

	if (window.surface_capabilities.currentExtent.width !=
		std::numeric_limits<uint32_t>::max())
	{
		swap_data.swap_extent = window.surface_capabilities.currentExtent;
	}
	else
	{
		int width {}, height {};
		glfwGetFramebufferSize(window.window_pointer, &width, &height);

		VkExtent2D actual_extent {static_cast<uint32_t>(width),
								  static_cast<uint32_t>(height)};

		actual_extent.width = std::clamp(
			actual_extent.width,
			window.surface_capabilities.minImageExtent.width,
			window.surface_capabilities.maxImageExtent.width);

		actual_extent.height = std::clamp(
			actual_extent.height,
			window.surface_capabilities.minImageExtent.height,
			window.surface_capabilities.maxImageExtent.height);

		swap_data.swap_extent = actual_extent;
	}

	uint32_t count {};
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev_data.physical_device,
										 window.surface,
										 &count,
										 nullptr);

	if (count == 0)
	{
		throw std::runtime_error("No surface formats found");
	}

	std::vector<VkSurfaceFormatKHR> surface_formats(count);
	rv = vkGetPhysicalDeviceSurfaceFormatsKHR(dev_data.physical_device,
											  window.surface,
											  &count,
											  surface_formats.data());

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get surface formats");
	}

	window.surface_format = surface_formats[0];
	for (const auto& available_format : surface_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			window.surface_format = available_format;
			break;
		}
	}

	count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(dev_data.physical_device,
											  window.surface,
											  &count,
											  nullptr);

	if (count == 0)
	{
		throw std::runtime_error("No surface present modes found");
	}

	std::vector<VkPresentModeKHR> present_modes(count);
	rv = vkGetPhysicalDeviceSurfacePresentModesKHR(dev_data.physical_device,
												   window.surface,
												   &count,
												   present_modes.data());

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get surface present modes");
	}

	swap_data.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& available_present_mode : present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			swap_data.present_mode = available_present_mode;
			break;
		}
	}
}
