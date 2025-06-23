#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <gsl/gsl>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_utility.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine.hpp>

std::string liboceanlight::utility::queue_flags_to_string(
	const VkQueueFlags& flags)
{
	std::stringstream formatted;

	std::map<unsigned int, std::string> flagbits {
		{VK_QUEUE_GRAPHICS_BIT, "Graphics"},
		{VK_QUEUE_COMPUTE_BIT, "Compute"},
		{VK_QUEUE_TRANSFER_BIT, "Transfer"},
		{VK_QUEUE_SPARSE_BINDING_BIT, "Sparsebinding"},
		{VK_QUEUE_PROTECTED_BIT, "Protected"},
		{VK_QUEUE_VIDEO_DECODE_BIT_KHR, "Video Decode"},
#ifdef VK_ENABLE_BETA_EXTENSIONS
		{VK_QUEUE_VIDEO_ENCODE, "Video Encode"},
#endif
		{VK_QUEUE_OPTICAL_FLOW_BIT_NV, "Optical Flow"}};

	for (const auto& flagbit : flagbits)
	{
		if (flags & flagbit.first)
			formatted << "|" << flagbit.second;
	}

	return formatted.str() + "|";
}

const std::vector<char> liboceanlight::utility::read_file(
	const std::string& path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file " + path);
	}

	const auto filesize {file.tellg()};
	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();

	return buffer;
}

void liboceanlight::utility::copy_buffer(VkBuffer src,
										 VkBuffer dst,
										 VkDeviceSize size)
{
	VkCommandBuffer cmd_buffer {engine_init::begin_single_time_cmds()};
	VkBufferCopy copy_region {};
	copy_region.size = size;
	vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);
	engine_init::end_single_time_cmds(cmd_buffer);
}

void liboceanlight::utility::copy_buffer_to_img(VkBuffer buff,
												VkImage img,
												uint32_t width,
												uint32_t height)
{
	VkCommandBuffer cmd_buffer {engine_init::begin_single_time_cmds()};
	VkBufferImageCopy region {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};
	vkCmdCopyBufferToImage(cmd_buffer,
						   buff,
						   img,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						   1,
						   &region);

	engine_init::end_single_time_cmds(cmd_buffer);
}

uint32_t liboceanlight::utility::find_mem_type(VkPhysicalDevice phys_device,
											   uint32_t type_filter,
											   VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(phys_device, &mem_props);

	for (uint32_t i {0}; i < mem_props.memoryTypeCount; ++i)
	{
		if ((type_filter & (1 << i)) &&
			(gsl::at(mem_props.memoryTypes, i).propertyFlags & flags) == flags)
		{
			return i;
		}
	}

	throw std::runtime_error("Couldn't find suitable memory type");
}

int liboceanlight::utility::test_func(int a, int b)
{
	return a + b;
}
