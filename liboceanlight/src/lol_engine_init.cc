#include <array>
#include <config.h>
#include <gsl/gsl>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>
#include <liboceanlight/lol_utility.hpp>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_swapchain.hpp>
#include <liboceanlight/lol_pipeline.hpp>
#include <liboceanlight/lol_instance.hpp>
#include <liboceanlight/lol_resource.hpp>

namespace fs = std::filesystem;
liboceanlight::engine_init::engine_init_data init_data;
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
	engine_init::create_cmd_pool();
	engine_init::create_depth_image();
	engine_init::create_framebuffers(dev_data.device,
									 pipe_data.render_pass,
									 swap_data);

	resource::texture_from_file(TEXTURE_PATH "viking_room.png",
								global_texture);
	resource::texture_img_view(global_texture.texture_img);
	resource::texture_sampler(
		dev_data.device_props.limits.maxSamplerAnisotropy,
		global_texture.texture_sampler);

	for (const auto& file : fs::directory_iterator(MODEL_PATH))
	{
		static int n {0};
		eng_data.model_list.emplace_back(MODEL_PATH +
										 file.path().filename().string());
		resource::model_from_obj(eng_data.model_list[n].path.c_str(),
								 eng_data.model_list[n]);
		std::cout << n << "\n";
		++n;
	}

	create_uniform_buffers(eng_data);
	create_descriptor_pool(eng_data);
	create_descriptor_sets(eng_data);
	create_cmd_buffer(eng_data);
	create_sync_objects(eng_data);

	return 1;
}

void liboceanlight::engine_init::create_framebuffers(
	VkDevice device,
	VkRenderPass render_pass,
	swapchain::swapchain_data& swapchain)
{
	size_t n {swap_data.image_views.size()};
	init_data.frame_buffers.resize(n);

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
								init_data.depth_img_view};
		c_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		c_info.pAttachments = attachments.data();

		rv = vkCreateFramebuffer(dev_data.device,
								 &c_info,
								 nullptr,
								 &init_data.frame_buffers[i]);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffers");
		}
	}
}

void liboceanlight::engine_init::create_cmd_pool()
{
	VkCommandPoolCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	c_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	c_info.queueFamilyIndex = dev_data.graphics_queue_index;

	VkResult rv = vkCreateCommandPool(dev_data.device,
									  &c_info,
									  nullptr,
									  &init_data.command_pool);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}
}

void liboceanlight::engine_init::create_depth_image()
{
	resource::image(swap_data.swap_extent.width,
					swap_data.swap_extent.height,
					init_data.depth_fmt,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					init_data.depth_img,
					init_data.depth_img_mem);

	init_data.depth_img_view = swapchain::create_image_view(
		dev_data.device,
		init_data.depth_img,
		init_data.depth_fmt,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	engine_init::transition_img_layout(
		init_data.depth_img,
		init_data.depth_fmt,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkCommandBuffer liboceanlight::engine_init::begin_single_time_cmds()
{
	VkCommandBufferAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = init_data.command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer cmd_buffer {};
	vkAllocateCommandBuffers(dev_data.device, &alloc_info, &cmd_buffer);

	VkCommandBufferBeginInfo begin_info {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd_buffer, &begin_info);
	return cmd_buffer;
}

void liboceanlight::engine_init::end_single_time_cmds(
	VkCommandBuffer cmd_buffer)
{
	vkEndCommandBuffer(cmd_buffer);

	VkSubmitInfo submit_info {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer;

	vkQueueSubmit(dev_data.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(dev_data.graphics_queue);
	vkFreeCommandBuffers(dev_data.device,
						 init_data.command_pool,
						 1,
						 &cmd_buffer);
}

bool has_stencil_component(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		   format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void liboceanlight::engine_init::transition_img_layout(
	VkImage img,
	VkFormat fmt,
	VkImageLayout old_layout,
	VkImageLayout new_layout)
{
	VkCommandBuffer cmd_buffer {begin_single_time_cmds()};
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

	engine_init::end_single_time_cmds(cmd_buffer);
}

void liboceanlight::engine::create_uniform_buffers(engine_data& eng_data)
{
	VkDeviceSize buff_size {sizeof(uniform_buffer_object)};

	for (auto i {0}; i < eng_data.max_frames_in_flight; ++i)
	{
		resource::buffer(buff_size,
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

void liboceanlight::engine::create_cmd_buffer(engine_data& eng_data)
{
	VkCommandBufferAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = init_data.command_pool;
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
