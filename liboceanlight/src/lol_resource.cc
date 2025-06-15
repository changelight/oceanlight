#include <vector>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_resource.hpp>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_engine_init.hpp>

void liboceanlight::resource::create_buffer(VkDeviceSize size,
											VkBufferUsageFlags usage,
											VkMemoryPropertyFlags props,
											VkBuffer& buff,
											VkDeviceMemory& buff_mem)
{
	VkBufferCreateInfo buff_info {};
	buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buff_info.size = size;
	buff_info.usage = usage;
	buff_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult rv = vkCreateBuffer(dev_data.device, &buff_info, nullptr, &buff);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer");
	}

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(dev_data.device, buff, &mem_reqs);

	uint32_t type_index = engine_init::find_mem_type(mem_reqs.memoryTypeBits,
													 props);

	VkMemoryAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = type_index;

	rv = vkAllocateMemory(dev_data.device, &alloc_info, nullptr, &buff_mem);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate vertex buffer memory");
	}

	vkBindBufferMemory(dev_data.device, buff, buff_mem, 0);
}

VkShaderModule liboceanlight::resource::create_shader(
	VkDevice device,
	const std::vector<char>& shader_code)
{
	VkShaderModuleCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_code.size();
	create_info.pCode = static_cast<const uint32_t*>(
		static_cast<const void*>(shader_code.data()));

	VkShaderModule shader_module {};
	auto rv = vkCreateShaderModule(device,
								   &create_info,
								   nullptr,
								   &shader_module);
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module");
	}

	return shader_module;
}

void liboceanlight::resource::create_image(uint32_t width,
										   uint32_t height,
										   VkFormat fmt,
										   VkImageTiling tiling,
										   VkImageUsageFlags usage,
										   VkMemoryPropertyFlags props,
										   VkImage& image,
										   VkDeviceMemory& image_mem)
{
	VkImageCreateInfo image_info {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(width);
	image_info.extent.height = static_cast<uint32_t>(height);
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = fmt;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.flags = 0;

	VkResult rv {};
	rv = vkCreateImage(dev_data.device, &image_info, nullptr, &image);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image");
	}

	VkMemoryRequirements mem_reqs {};
	vkGetImageMemoryRequirements(dev_data.device, image, &mem_reqs);

	VkMemoryAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = engine_init::find_mem_type(
		mem_reqs.memoryTypeBits,
		props);

	rv = vkAllocateMemory(dev_data.device, &alloc_info, nullptr, &image_mem);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(dev_data.device, image, image_mem, 0);
}

/*
void liboceanlight::engine::create_vk_resource()
{

}

void liboceanlight::engine::create_image(engine_data& eng_data,
										 uint32_t width,
										 uint32_t height,
										 VkFormat fmt,
										 VkImageTiling tiling,
										 VkImageUsageFlags usage,
										 VkMemoryPropertyFlags props,
										 VkImage& image,
										 VkDeviceMemory& image_mem)
{
	VkImageCreateInfo image_info {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(width);
	image_info.extent.height = static_cast<uint32_t>(height);
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = fmt;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.flags = 0;

	VkResult rv {};
	rv = vkCreateImage(eng_data.logical_device, &image_info, nullptr, &image);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image");
	}

	VkMemoryRequirements mem_reqs {};
	vkGetImageMemoryRequirements(eng_data.logical_device, image, &mem_reqs);

	VkMemoryAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = find_mem_type(eng_data,
											   mem_reqs.memoryTypeBits,
											   props);

	rv = vkAllocateMemory(eng_data.logical_device,
						  &alloc_info,
						  nullptr,
						  &image_mem);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(eng_data.logical_device, image, image_mem, 0);
}
*/
