#include <cstdint>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <gsl/gsl>
#include <liboceanlight/lol_resource.hpp>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_pipeline.hpp>
#include <liboceanlight/lol_utility.hpp>

void liboceanlight::resource::buffer(VkDeviceSize size,
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

	uint32_t type_index = utility::find_mem_type(dev_data.phys_device,
												 mem_reqs.memoryTypeBits,
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

	PFN_vkSetDeviceMemoryPriorityEXT vkSetDeviceMemoryPriorityEXT_ {nullptr};
	vkSetDeviceMemoryPriorityEXT_ = (PFN_vkSetDeviceMemoryPriorityEXT)
		vkGetDeviceProcAddr(dev_data.device, "vkSetDeviceMemoryPriorityEXT");

	if (vkSetDeviceMemoryPriorityEXT_)
	{
		vkSetDeviceMemoryPriorityEXT_(dev_data.device, buff_mem, 1.0);
	}

	vkBindBufferMemory(dev_data.device, buff, buff_mem, 0);
}

VkShaderModule liboceanlight::resource::shader(
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

void liboceanlight::resource::image(uint32_t width,
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
	alloc_info.memoryTypeIndex = utility::find_mem_type(
		dev_data.phys_device,
		mem_reqs.memoryTypeBits,
		props);

	rv = vkAllocateMemory(dev_data.device, &alloc_info, nullptr, &image_mem);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory");
	}

	PFN_vkSetDeviceMemoryPriorityEXT vkSetDeviceMemoryPriorityEXT_ {nullptr};
	vkSetDeviceMemoryPriorityEXT_ = (PFN_vkSetDeviceMemoryPriorityEXT)
		vkGetDeviceProcAddr(dev_data.device, "vkSetDeviceMemoryPriorityEXT");

	if (vkSetDeviceMemoryPriorityEXT_)
	{
		vkSetDeviceMemoryPriorityEXT_(dev_data.device, image_mem, 1.0);
	}

	vkBindImageMemory(dev_data.device, image, image_mem, 0);
}

void liboceanlight::resource::texture_from_file(
	const char* path,
	liboceanlight::texture::lol_texture& texture)
{
	int width {}, height {}, channels {}, bytes_per_component {STBI_rgb_alpha};

	stbi_uc* pixels {nullptr};
	pixels = stbi_load(path, &width, &height, &channels, bytes_per_component);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image");
	}

	VkDeviceSize img_size {
		static_cast<VkDeviceSize>(width * height * bytes_per_component)};
	VkBuffer staging_buff {nullptr};
	VkDeviceMemory staging_buff_mem {nullptr};

	resource::buffer(img_size,
					 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 staging_buff,
					 staging_buff_mem);

	void* data {nullptr};
	vkMapMemory(dev_data.device, staging_buff_mem, 0, img_size, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(img_size));
	vkUnmapMemory(dev_data.device, staging_buff_mem);
	stbi_image_free(pixels);

	resource::image(width,
					height,
					VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_DST_BIT |
						VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					texture.texture_img,
					texture.texture_img_mem);

	engine_init::transition_img_layout(texture.texture_img,
									   VK_FORMAT_R8G8B8A8_SRGB,
									   VK_IMAGE_LAYOUT_UNDEFINED,
									   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	utility::copy_buffer_to_img(staging_buff,
								texture.texture_img,
								static_cast<uint32_t>(width),
								static_cast<uint32_t>(height));

	engine_init::transition_img_layout(
		texture.texture_img,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(dev_data.device, staging_buff, nullptr);
	vkFreeMemory(dev_data.device, staging_buff_mem, nullptr);
}

void liboceanlight::resource::texture_img_view(VkImage& texture_image)
{
	global_texture.texture_img_view = swapchain::create_image_view(
		dev_data.device,
		texture_image,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT);
}

void liboceanlight::resource::texture_sampler(float max_anisotropy,
											  VkSampler& sampler)
{
	VkSamplerCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	c_info.magFilter = VK_FILTER_LINEAR;
	c_info.minFilter = VK_FILTER_LINEAR;
	c_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.anisotropyEnable = VK_TRUE;
	c_info.maxAnisotropy = max_anisotropy;
	c_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	c_info.unnormalizedCoordinates = VK_FALSE;
	c_info.compareEnable = VK_FALSE;
	c_info.compareOp = VK_COMPARE_OP_ALWAYS;
	c_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	c_info.mipLodBias = 0.0f;
	c_info.minLod = 0.0f;
	c_info.maxLod = 0.0f;

	VkResult rv = vkCreateSampler(dev_data.device, &c_info, nullptr, &sampler);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}
}

void liboceanlight::resource::model_from_obj(const char* path,
											 models::lol_model& model)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	bool rv;
	rv = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);

	if (!rv)
	{
		throw std::runtime_error("Failed to load model " + std::string(path) +
								 "\n" + warn + err);
	}

	std::unordered_map<engine::vertex, uint32_t> unique_vertices {};

	for (const auto& shape : shapes)
	{
		for (const auto& i : shape.mesh.indices)
		{
			engine::vertex v {};

			if (i.vertex_index >= 0)
			{
				v.pos = {attrib.vertices[3 * i.vertex_index + 0],
						 attrib.vertices[3 * i.vertex_index + 1],
						 attrib.vertices[3 * i.vertex_index + 2]};
			}

			if (i.normal_index >= 0)
			{
				v.normal = {attrib.normals[3 * i.normal_index + 0],
							attrib.normals[3 * i.normal_index + 1],
							attrib.normals[3 * i.normal_index + 2]};
			}

			if (i.texcoord_index >= 0)
			{
				v.texcoord = {attrib.texcoords[2 * i.texcoord_index + 0],
							  1.0f -
								  attrib.texcoords[2 * i.texcoord_index + 1]};
			}

			auto color_index = 3 * i.vertex_index + 2;
			if (color_index < attrib.colors.size())
			{
				v.color = {attrib.colors[color_index - 2],
						   attrib.colors[color_index - 1],
						   attrib.colors[color_index - 0]};
			}
			else
			{
				v.color = {1.0f, 1.0f, 1.0f};
			}

			if (unique_vertices.count(v) == 0)
			{
				unique_vertices[v] = static_cast<uint32_t>(
					model.vertices.size());
				model.vertices.push_back(v);
			}
			model.indices.push_back(unique_vertices[v]);
		}
	}

	engine::upload_buffer(model.vertices.data(),
						  sizeof(model.vertices[0]) * model.vertices.size(),
						  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						  model.vertex_buffer,
						  model.vertex_buffer_mem);

	engine::upload_buffer(model.indices.data(),
						  sizeof(model.indices[0]) * model.indices.size(),
						  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
						  model.index_buffer,
						  model.index_buffer_mem);
}

void liboceanlight::resource::uniform_buffer(VkBuffer& buffer,
											 VkDeviceMemory& memory,
											 void** mapped)
{
	VkDeviceSize buff_size {sizeof(engine::uniform_buffer_object)};

	resource::buffer(buff_size,
					 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					 buffer,
					 memory);

	vkMapMemory(dev_data.device, memory, 0, buff_size, 0, mapped);
}

void liboceanlight::resource::descriptor_pool(VkDescriptorPoolSize* pool_sizes,
											  uint32_t size_count,
											  engine::engine_data& eng_data)
{
	VkDescriptorPoolCreateInfo pool_info {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = size_count;
	pool_info.pPoolSizes = pool_sizes;
	pool_info.maxSets = static_cast<uint32_t>(eng_data.max_frames_in_flight);

	VkResult rv = vkCreateDescriptorPool(dev_data.device,
										 &pool_info,
										 nullptr,
										 &eng_data.descriptor_pool);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

void liboceanlight::resource::descriptor_set(engine::engine_data& eng_data)
{
	std::vector<VkDescriptorSetLayout> layouts(
		eng_data.max_frames_in_flight,
		pipe_data.descriptor_set_layout);

	VkDescriptorSetAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = eng_data.descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(
		eng_data.max_frames_in_flight);
	alloc_info.pSetLayouts = layouts.data();

	VkResult rv = vkAllocateDescriptorSets(dev_data.device,
										   &alloc_info,
										   eng_data.descriptor_sets.data());

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	for (int i {0}; i < eng_data.max_frames_in_flight; ++i)
	{
		VkDescriptorBufferInfo buff_info {};
		buff_info.buffer = gsl::at(eng_data.uniform_buffers, i);
		buff_info.offset = 0;
		buff_info.range = sizeof(engine::uniform_buffer_object);

		VkDescriptorImageInfo image_info {};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = global_texture.texture_img_view;
		image_info.sampler = global_texture.texture_sampler;

		std::array<VkWriteDescriptorSet, 2> descriptor_writes {};
		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = gsl::at(eng_data.descriptor_sets, i);
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType =
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pBufferInfo = &buff_info;

		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[1].dstSet = gsl::at(eng_data.descriptor_sets, i);
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].dstArrayElement = 0;
		descriptor_writes[1].descriptorType =
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[1].descriptorCount = 1;
		descriptor_writes[1].pImageInfo = &image_info;

		vkUpdateDescriptorSets(dev_data.device,
							   static_cast<uint32_t>(descriptor_writes.size()),
							   descriptor_writes.data(),
							   0,
							   nullptr);
	}
}
