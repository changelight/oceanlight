#include <stdexcept>
#include <algorithm>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_swapchain.hpp>
#include <liboceanlight/lol_device.hpp>

liboceanlight::swapchain::swapchain_data swap_data;

void liboceanlight::swapchain::init_swapchain(liboceanlight::window& window,
											  VkPhysicalDevice physical_device,
											  VkDevice logical_device)
{
	VkResult rv = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physical_device,
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
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
										 window.surface,
										 &count,
										 nullptr);

	if (count == 0)
	{
		throw std::runtime_error("No surface formats found");
	}

	std::vector<VkSurfaceFormatKHR> surface_formats(count);
	rv = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
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
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
											  window.surface,
											  &count,
											  nullptr);

	if (count == 0)
	{
		throw std::runtime_error("No surface present modes found");
	}

	std::vector<VkPresentModeKHR> present_modes(count);
	rv = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
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

void liboceanlight::swapchain::create_swapchain(liboceanlight::window& w,
												VkDevice logical_device)
{
	uint32_t img_count = w.surface_capabilities.minImageCount + 1;
	uint32_t max_img_count = w.surface_capabilities.maxImageCount;

	if (max_img_count > 0)
	{
		img_count = std::max(img_count, max_img_count);
	}
	else
	{
		throw std::runtime_error("Error: Max swapchain image count is 0.");
	}

	VkSwapchainCreateInfoKHR c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	c_info.surface = w.surface;
	c_info.minImageCount = img_count;
	c_info.imageFormat = w.surface_format.format;
	c_info.imageColorSpace = w.surface_format.colorSpace;
	c_info.imageExtent = swap_data.swap_extent;
	c_info.imageArrayLayers = 1;
	c_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	c_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	c_info.queueFamilyIndexCount = 0;
	c_info.pQueueFamilyIndices = nullptr;
	c_info.preTransform = w.surface_capabilities.currentTransform;
	c_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	c_info.presentMode = swap_data.present_mode;
	c_info.clipped = VK_TRUE;
	c_info.oldSwapchain = VK_NULL_HANDLE;

	VkResult rv = vkCreateSwapchainKHR(logical_device,
									   &c_info,
									   nullptr,
									   &swap_data.swap_chain);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain");
	}

	img_count = 0;
	vkGetSwapchainImagesKHR(logical_device,
							swap_data.swap_chain,
							&img_count,
							nullptr);

	swap_data.images.resize(img_count);
	vkGetSwapchainImagesKHR(logical_device,
							swap_data.swap_chain,
							&img_count,
							swap_data.images.data());
}

VkImageView liboceanlight::swapchain::create_image_view(
	VkDevice logical_device,
	VkImage img,
	VkFormat fmt,
	VkImageAspectFlags flags)
{
	VkImageViewCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	c_info.image = img;
	c_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	c_info.format = fmt;
	c_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.subresourceRange.aspectMask = flags;
	c_info.subresourceRange.baseMipLevel = 0;
	c_info.subresourceRange.levelCount = 1;
	c_info.subresourceRange.baseArrayLayer = 0;
	c_info.subresourceRange.layerCount = 1;

	VkImageView img_view {};
	VkResult rv = vkCreateImageView(logical_device,
									&c_info,
									nullptr,
									&img_view);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image view");
	}

	return img_view;
}

void liboceanlight::swapchain::create_image_views(liboceanlight::window& w,
												  VkDevice logical_device)
{
	const std::vector<int>::size_type n = swap_data.images.size();
	swap_data.image_views.resize(n);
	for (std::vector<int>::size_type i {0}; i < n; ++i)
	{
		swap_data.image_views[i] = create_image_view(
			logical_device,
			swap_data.images[i],
			w.surface_format.format,
			VK_IMAGE_ASPECT_COLOR_BIT);
	}
}
