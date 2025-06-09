#include "liboceanlight/lol_window.hpp"
#include <array>
#include <config.h>
#include <cstring>
#include <filesystem>
#include <gsl/gsl>
#include <iostream>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>
#include <liboceanlight/lol_utility.hpp>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_swapchain.hpp>
#include <liboceanlight/lol_pipeline.hpp>
#include <stb_image.h>
#include <stdexcept>
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>
#include <liboceanlight/lol_instance.hpp>

namespace fs = std::filesystem;
using namespace liboceanlight::engine;

int liboceanlight::engine::init(liboceanlight::window& window,
								engine_data& eng_data)
{
	instance::create_instance();
	device::create_physical_device(inst_data.vulkan_instance);
	window.create_surface(inst_data.vulkan_instance);
	device::check_device_queue_support(window.surface);
	device::create_logical_device();
	swapchain::init_swapchain(window, dev_data.phys_device, dev_data.device);
	swapchain::create_swapchain(window, dev_data.device);
	swapchain::create_image_views(window, dev_data.device);
	pipeline::create_render_pass(window, dev_data.device);
	pipeline::create_descriptor_set_layout(dev_data.device);
	pipeline::create_pipeline(dev_data.device, swap_data.swap_extent);

	create_cmd_pool(eng_data);
	create_depth_resources(eng_data);
	create_framebuffers(eng_data);
	create_texture_img(eng_data);
	create_texture_img_view(eng_data);
	create_texture_sampler(eng_data);

	load_models(eng_data);
	create_vertex_buffers(eng_data);
	create_index_buffers(eng_data);
	create_uniform_buffers(eng_data);
	create_descriptor_pool(eng_data);
	create_descriptor_sets(eng_data);
	create_cmd_buffer(eng_data);
	create_sync_objects(eng_data);

	return 1;
}

void liboceanlight::engine::create_framebuffers(engine_data& eng_data)
{
	size_t n {swap_data.image_views.size()};
	eng_data.frame_buffers.resize(n);

	VkFramebufferCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	c_info.renderPass = pipe_data.render_pass;
	c_info.width = swap_data.swap_extent.width;
	c_info.height = swap_data.swap_extent.height;
	c_info.layers = 1;

	VkResult rv {};
	for (size_t i {0}; i < n; ++i)
	{
		std::array attachments {swap_data.image_views[i],
								eng_data.depth_img_view};
		c_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		c_info.pAttachments = attachments.data();

		rv = vkCreateFramebuffer(dev_data.device,
								 &c_info,
								 nullptr,
								 &eng_data.frame_buffers[i]);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffers");
		}
	}
}

void liboceanlight::engine::create_cmd_pool(engine_data& eng_data)
{
	VkCommandPoolCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	c_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	c_info.queueFamilyIndex = dev_data.graphics_queue_index;

	VkResult rv = vkCreateCommandPool(dev_data.device,
									  &c_info,
									  nullptr,
									  &eng_data.command_pool);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}
}

void liboceanlight::engine::create_depth_resources(engine_data& eng_data)
{
	create_image(eng_data,
				 swap_data.swap_extent.width,
				 swap_data.swap_extent.height,
				 eng_data.depth_fmt,
				 VK_IMAGE_TILING_OPTIMAL,
				 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 eng_data.depth_img,
				 eng_data.depth_img_mem);

	eng_data.depth_img_view = swapchain::create_image_view(
		dev_data.device,
		eng_data.depth_img,
		eng_data.depth_fmt,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	transition_img_layout(eng_data,
						  eng_data.depth_img,
						  eng_data.depth_fmt,
						  VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void liboceanlight::engine::create_texture_img(engine_data& eng_data)
{
	int width {}, height {}, channels {}, bytes_per_pixel {STBI_rgb_alpha};
	const char* path {TEXTURE_PATH "viking_room.png"};

	stbi_uc* pixels {nullptr};
	pixels = stbi_load(path, &width, &height, &channels, bytes_per_pixel);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image");
	}

	VkDeviceSize img_size {
		static_cast<VkDeviceSize>(width * height * bytes_per_pixel)};
	VkBuffer staging_buff {nullptr};
	VkDeviceMemory staging_buff_mem {nullptr};

	create_buffer(eng_data,
				  img_size,
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

	create_image(eng_data,
				 width,
				 height,
				 VK_FORMAT_R8G8B8A8_SRGB,
				 VK_IMAGE_TILING_OPTIMAL,
				 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 eng_data.texture_img,
				 eng_data.texture_img_mem);

	transition_img_layout(eng_data,
						  eng_data.texture_img,
						  VK_FORMAT_R8G8B8A8_SRGB,
						  VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copy_buffer_to_img(eng_data,
					   staging_buff,
					   eng_data.texture_img,
					   static_cast<uint32_t>(width),
					   static_cast<uint32_t>(height));

	transition_img_layout(eng_data,
						  eng_data.texture_img,
						  VK_FORMAT_R8G8B8A8_SRGB,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(dev_data.device, staging_buff, nullptr);
	vkFreeMemory(dev_data.device, staging_buff_mem, nullptr);
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
	alloc_info.memoryTypeIndex = find_mem_type(eng_data,
											   mem_reqs.memoryTypeBits,
											   props);

	rv = vkAllocateMemory(dev_data.device, &alloc_info, nullptr, &image_mem);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(dev_data.device, image, image_mem, 0);
}

VkCommandBuffer liboceanlight::engine::begin_single_time_cmds(
	engine_data& eng_data)
{
	VkCommandBufferAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = eng_data.command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer cmd_buffer {};
	vkAllocateCommandBuffers(dev_data.device, &alloc_info, &cmd_buffer);

	VkCommandBufferBeginInfo begin_info {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd_buffer, &begin_info);
	return cmd_buffer;
}

void liboceanlight::engine::end_single_time_cmds(engine_data& eng_data,
												 VkCommandBuffer& cmd_buffer)
{
	vkEndCommandBuffer(cmd_buffer);

	VkSubmitInfo submit_info {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer;

	vkQueueSubmit(dev_data.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(dev_data.graphics_queue);
	vkFreeCommandBuffers(dev_data.device,
						 eng_data.command_pool,
						 1,
						 &cmd_buffer);
}

bool has_stencil_component(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		   format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void liboceanlight::engine::transition_img_layout(engine_data& eng_data,
												  VkImage img,
												  VkFormat fmt,
												  VkImageLayout old_layout,
												  VkImageLayout new_layout)
{
	VkCommandBuffer cmd_buffer {begin_single_time_cmds(eng_data)};
	VkImageMemoryBarrier barrier {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = img;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	VkPipelineStageFlags src_stage {}, dst_stage {};
	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
		new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			 new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
			 new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
								VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::runtime_error("Unsupported layout transition");
	}

	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (has_stencil_component(fmt))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(cmd_buffer,
						 src_stage,
						 dst_stage,
						 0,
						 0,
						 nullptr,
						 0,
						 nullptr,
						 1,
						 &barrier);

	end_single_time_cmds(eng_data, cmd_buffer);
}

void liboceanlight::engine::copy_buffer_to_img(engine_data& eng_data,
											   VkBuffer buff,
											   VkImage img,
											   uint32_t width,
											   uint32_t height)
{
	VkCommandBuffer cmd_buffer {begin_single_time_cmds(eng_data)};
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
	end_single_time_cmds(eng_data, cmd_buffer);
}

void liboceanlight::engine::create_texture_img_view(engine_data& eng_data)
{
	eng_data.texture_img_view = swapchain::create_image_view(
		dev_data.device,
		eng_data.texture_img,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT);
}

void liboceanlight::engine::create_texture_sampler(engine_data& eng_data)
{
	VkSamplerCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	c_info.magFilter = VK_FILTER_LINEAR;
	c_info.minFilter = VK_FILTER_LINEAR;
	c_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.anisotropyEnable = VK_TRUE;
	c_info.maxAnisotropy = dev_data.device_props.limits.maxSamplerAnisotropy;
	c_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	c_info.unnormalizedCoordinates = VK_FALSE;
	c_info.compareEnable = VK_FALSE;
	c_info.compareOp = VK_COMPARE_OP_ALWAYS;
	c_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	c_info.mipLodBias = 0.0f;
	c_info.minLod = 0.0f;
	c_info.maxLod = 0.0f;

	VkResult rv = vkCreateSampler(dev_data.device,
								  &c_info,
								  nullptr,
								  &eng_data.texture_sampler);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}
}

void liboceanlight::engine::load_models(engine_data& eng_data)
{
	for (const auto& file : fs::directory_iterator(MODEL_PATH))
	{
		eng_data.model_list.emplace_back(file.path().filename().string());
		std::cout << "Loaded model " << file.path().filename() << "\n";
	}

	for (auto& model : eng_data.model_list)
	{
		std::string model_file {MODEL_PATH + model.name};
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		auto rv = tinyobj::LoadObj(&attrib,
								   &shapes,
								   &materials,
								   &warn,
								   &err,
								   model_file.c_str());

		if (!rv)
		{
			throw std::runtime_error("Failed to load model " + model_file +
									 "\n" + warn + err);
		}

		std::unordered_map<vertex, uint32_t> unique_vertices {};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				vertex vertex {};

				vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
							  attrib.vertices[3 * index.vertex_index + 1],
							  attrib.vertices[3 * index.vertex_index + 2]};

				vertex.texcoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

				vertex.color = {1.0f, 1.0f, 1.0f};

				if (unique_vertices.count(vertex) == 0)
				{
					unique_vertices[vertex] = static_cast<uint32_t>(
						model.vertices.size());
					model.vertices.push_back(vertex);
				}

				model.indices.push_back(unique_vertices[vertex]);
			}
		}
	}
}

void liboceanlight::engine::create_vertex_buffers(engine_data& eng_data)
{
	for (auto& model : eng_data.model_list)
	{
		upload_buffer(eng_data,
					  model.vertices.data(),
					  sizeof(model.vertices[0]) * model.vertices.size(),
					  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					  model.vertex_buffer,
					  model.vertex_buffer_mem);
	}
}

void liboceanlight::engine::create_index_buffers(engine_data& eng_data)
{
	for (auto& model : eng_data.model_list)
	{
		upload_buffer(eng_data,
					  model.indices.data(),
					  sizeof(model.indices[0]) * model.indices.size(),
					  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					  model.index_buffer,
					  model.index_buffer_mem);
	}
}

void liboceanlight::engine::create_uniform_buffers(engine_data& eng_data)
{
	VkDeviceSize buff_size {sizeof(uniform_buffer_object)};

	for (auto i {0}; i < eng_data.max_frames_in_flight; ++i)
	{
		create_buffer(eng_data,
					  buff_size,
					  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					  gsl::at(eng_data.uniform_buffers, i),
					  gsl::at(eng_data.uniform_buffers_mem, i));

		vkMapMemory(dev_data.device,
					gsl::at(eng_data.uniform_buffers_mem, i),
					0,
					buff_size,
					0,
					&gsl::at(eng_data.uniform_buffers_mapped, i));
	}
}

void liboceanlight::engine::create_descriptor_pool(engine_data& eng_data)
{
	std::array<VkDescriptorPoolSize, 2> pool_sizes {};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(
		eng_data.max_frames_in_flight);
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(
		eng_data.max_frames_in_flight);

	VkDescriptorPoolCreateInfo pool_info {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();
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

void liboceanlight::engine::create_descriptor_sets(engine_data& eng_data)
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
		buff_info.range = sizeof(uniform_buffer_object);

		VkDescriptorImageInfo image_info {};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = eng_data.texture_img_view;
		image_info.sampler = eng_data.texture_sampler;

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

void liboceanlight::engine::create_buffer(engine_data& eng_data,
										  VkDeviceSize size,
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

	uint32_t type_index = find_mem_type(eng_data,
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

	vkBindBufferMemory(dev_data.device, buff, buff_mem, 0);
}

void liboceanlight::engine::copy_buffer(engine_data& eng_data,
										VkBuffer src,
										VkBuffer dst,
										VkDeviceSize size)
{
	VkCommandBuffer cmd_buffer {begin_single_time_cmds(eng_data)};
	VkBufferCopy copy_region {};
	copy_region.size = size;
	vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);
	end_single_time_cmds(eng_data, cmd_buffer);
}

uint32_t liboceanlight::engine::find_mem_type(engine_data& eng_data,
											  uint32_t type_filter,
											  VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(dev_data.phys_device, &mem_props);

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

void liboceanlight::engine::create_cmd_buffer(engine_data& eng_data)
{
	VkCommandBufferAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = eng_data.command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)eng_data.max_frames_in_flight;

	VkResult rv = vkAllocateCommandBuffers(dev_data.device,
										   &alloc_info,
										   eng_data.command_buffers.data());
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer");
	}
}

void liboceanlight::engine::create_sync_objects(engine_data& eng_data)
{
	VkSemaphoreCreateInfo sem_info {};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult rv {};
	for (int i {0}; i < eng_data.max_frames_in_flight; ++i)
	{
		rv = vkCreateSemaphore(dev_data.device,
							   &sem_info,
							   nullptr,
							   &gsl::at(eng_data.signal_sems, i));

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create signal semaphore");
		}

		rv = vkCreateSemaphore(dev_data.device,
							   &sem_info,
							   nullptr,
							   &gsl::at(eng_data.wait_sems, i));

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create wait semaphore");
		}

		rv = vkCreateFence(dev_data.device,
						   &fence_info,
						   nullptr,
						   &gsl::at(eng_data.in_flight_fences, i));

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create fence");
		}
	}
}
