#include <GLFW/glfw3.h>
#include <chrono>
#include <config.h>
#include <cstring>
#include <gsl/gsl>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>
#include <liboceanlight/lol_utility.hpp>
#include <liboceanlight/lol_window.hpp>

using namespace liboceanlight::engine;
double scroll_offset {0.0f}, cursor_posx {0.0f}, cursor_posy {0.0f};
lol_camera camera;

void liboceanlight::engine::start(liboceanlight::window& window,
								  engine_data& eng_data)
{
	init(window, eng_data);
	run(window, eng_data);
}

void liboceanlight::engine::run(liboceanlight::window& window,
								engine_data& eng_data)
{
	auto current_time {std::chrono::high_resolution_clock::now()};
	while (!window.should_close())
	{
		auto new_time {std::chrono::high_resolution_clock::now()};
		double dt {
			std::chrono::duration<double, std::milli>(new_time - current_time)
				.count()};
		current_time = new_time;
		glfwPollEvents();
		draw_frame(window, eng_data, dt);
		current_time += (new_time - current_time);
	}

	vkDeviceWaitIdle(eng_data.logical_device);
}

void liboceanlight::engine::draw_frame(liboceanlight::window& window,
									   engine_data& eng_data,
									   double dt)
{
	vkWaitForFences(
		eng_data.logical_device,
		1,
		&gsl::at(eng_data.in_flight_fences, eng_data.current_frame),
		VK_TRUE,
		UINT64_MAX);

	uint32_t image_index {};
	VkResult rv = vkAcquireNextImageKHR(
		eng_data.logical_device,
		eng_data.swap_chain,
		UINT64_MAX,
		gsl::at(eng_data.wait_sems, eng_data.current_frame),
		VK_NULL_HANDLE,
		&image_index);

	if (rv == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate_swapchain(window, eng_data);
		return;
	}
	else if (rv != VK_SUCCESS && rv != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	vkResetFences(eng_data.logical_device,
				  1,
				  &gsl::at(eng_data.in_flight_fences, eng_data.current_frame));

	vkResetCommandBuffer(
		gsl::at(eng_data.command_buffers, eng_data.current_frame),
		0);
	record_cmd_buffer(
		eng_data,
		gsl::at(eng_data.command_buffers, eng_data.current_frame),
		image_index);

	update_uniform_buffer(eng_data, window, eng_data.current_frame, dt);

	VkSubmitInfo submit_info {};
	std::array signal {gsl::at(eng_data.signal_sems, eng_data.current_frame)};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal.data();

	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	std::array wait {gsl::at(eng_data.wait_sems, eng_data.current_frame)};
	const VkPipelineStageFlags wait_stages {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait.data();
	submit_info.pWaitDstStageMask = &wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &gsl::at(eng_data.command_buffers,
										   eng_data.current_frame);

	rv = vkQueueSubmit(
		eng_data.graphics_queue,
		1,
		&submit_info,
		gsl::at(eng_data.in_flight_fences, eng_data.current_frame));

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}

	VkPresentInfoKHR present_info {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal.data();

	const VkSwapchainKHR swap_chains {eng_data.swap_chain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swap_chains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr;

	rv = vkQueuePresentKHR(eng_data.graphics_queue, &present_info);

	if (rv == VK_ERROR_OUT_OF_DATE_KHR | rv == VK_SUBOPTIMAL_KHR |
		window.framebuffer_resized)
	{
		window.framebuffer_resized = false;
		recreate_swapchain(window, eng_data);
	}
	else if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present frame");
	}

	eng_data.current_frame = (eng_data.current_frame + 1) %
							 eng_data.max_frames_in_flight;
}

void liboceanlight::engine::record_cmd_buffer(engine_data& eng_data,
											  VkCommandBuffer& cmd_buffer,
											  uint32_t image_index)
{
	VkCommandBufferBeginInfo begin_info {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = nullptr;

	VkResult rv = vkBeginCommandBuffer(cmd_buffer, &begin_info);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer");
	}

	VkClearValue color_clear_val {{0.0f, 0.0f, 0.0f, 1.0f}};
	VkClearValue depth_stencil_clear_val {1.0f, 0};
	std::array<VkClearValue, 2> clear_values {color_clear_val,
											  depth_stencil_clear_val};
	VkRenderPassBeginInfo pass_info {};
	pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	pass_info.renderPass = eng_data.render_pass;
	pass_info.framebuffer = eng_data.frame_buffers[image_index];
	pass_info.renderArea.offset = {0, 0};
	pass_info.renderArea.extent = eng_data.swap_extent;
	pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	pass_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(cmd_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd_buffer,
					  VK_PIPELINE_BIND_POINT_GRAPHICS,
					  eng_data.graphics_pipeline);

	vkCmdBindDescriptorSets(
		cmd_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		eng_data.pipeline_layout,
		0,
		1,
		&gsl::at(eng_data.descriptor_sets, eng_data.current_frame),
		0,
		nullptr);

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(eng_data.swap_extent.width);
	viewport.height = static_cast<float>(eng_data.swap_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

	VkRect2D scissor {};
	scissor.offset = {0, 0};
	scissor.extent = eng_data.swap_extent;
	vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

	for (auto& model : eng_data.model_list)
	{
		std::array vertex_buffers {model.vertex_buffer};
		VkDeviceSize offsets {0};
		vkCmdBindVertexBuffers(cmd_buffer,
							   0,
							   1,
							   vertex_buffers.data(),
							   &offsets);
		vkCmdBindIndexBuffer(cmd_buffer,
							 model.index_buffer,
							 0,
							 VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd_buffer,
						 static_cast<uint32_t>(model.indices.size()),
						 1,
						 0,
						 0,
						 0);
	}

	vkCmdEndRenderPass(cmd_buffer);

	rv = vkEndCommandBuffer(cmd_buffer);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer");
	}
}

void liboceanlight::engine::update_uniform_buffer(
	engine_data& eng_data,
	liboceanlight::window& window,
	uint32_t current_image,
	double dt)
{
	static auto start_time {std::chrono::high_resolution_clock::now()};
	auto current_time {std::chrono::high_resolution_clock::now()};
	float time {std::chrono::duration<float, std::chrono::seconds::period>(
					current_time - start_time)
					.count()};

	uniform_buffer_object ubo {};

	static glm::vec3 scale {1.0f, 1.0f, 1.0f};
	static float scale_quota {0.0f};
	const float scale_step {0.0025f * static_cast<float>(dt)};

	if (scroll_offset != 0.0f)
	{
		scale_quota += static_cast<float>(scroll_offset / 8.0f);
		scroll_offset = 0.0f;
	}

	if (scale_quota > 0.0f + scale_step)
	{
		scale += scale_step;
		scale_quota -= scale_step;
	}
	else if (scale_quota < 0.0f - scale_step)
	{
		scale -= scale_step;
		scale_quota += scale_step;
	}

	ubo.model = glm::scale(ubo.model, scale);
	const float angle {35.0f}, initial_angle {-90.0f + -45.0f};

	ubo.model = glm::rotate(ubo.model,
							glm::radians(initial_angle),
							glm::vec3(0.0f, 1.0f, 0.0f));

	ubo.model = glm::rotate(ubo.model,
							sin(time) * glm::radians(angle),
							glm::vec3(0.0f, 1.0f, 0.0f));

	update_camera(window, static_cast<float>(dt));
	ubo.view = glm::lookAt(camera.eye, camera.eye + camera.center, camera.up);

	const float degrees {60.0f}, zfar {1000.0f}, znear {0.1f};
	ubo.proj = glm::perspective(
		glm::radians(degrees),
		static_cast<float>(eng_data.swap_extent.width) /
			static_cast<float>(eng_data.swap_extent.height),
		znear,
		zfar);
	ubo.proj[1][1] *= -1;

	memcpy(gsl::at(eng_data.uniform_buffers_mapped, current_image),
		   &ubo,
		   sizeof(ubo));
}

void liboceanlight::engine::update_camera(liboceanlight::window& window,
										  float dt)
{
	int state = glfwGetKey(window.window_pointer, GLFW_KEY_W);
	if (state == GLFW_PRESS)
	{
		camera.eye += camera.movement_speed * camera.center *
					  static_cast<float>(dt);
	}

	state = glfwGetKey(window.window_pointer, GLFW_KEY_A);
	if (state == GLFW_PRESS)
	{
		camera.eye -= camera.movement_speed *
					  glm::normalize(glm::cross(camera.center, camera.up)) *
					  static_cast<float>(dt);
	}

	state = glfwGetKey(window.window_pointer, GLFW_KEY_S);
	if (state == GLFW_PRESS)
	{
		camera.eye -= camera.movement_speed * camera.center *
					  static_cast<float>(dt);
	}

	state = glfwGetKey(window.window_pointer, GLFW_KEY_D);
	if (state == GLFW_PRESS)
	{
		camera.eye += camera.movement_speed *
					  glm::normalize(glm::cross(camera.center, camera.up)) *
					  static_cast<float>(dt);
	}
}

void liboceanlight::engine::upload_buffer(engine_data& eng_data,
										  const void* buff,
										  VkDeviceSize buff_size,
										  VkBufferUsageFlagBits usage,
										  VkBuffer& dst,
										  VkDeviceMemory& dst_mem)
{
	if ((!buff) | (buff_size == 0))
	{
		throw std::runtime_error("Invalid buffer to be uploaded");
	}

	VkBuffer staging_buff {nullptr};
	VkDeviceMemory staging_buff_mem {nullptr};
	create_buffer(eng_data,
				  buff_size,
				  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				  staging_buff,
				  staging_buff_mem);

	void* data {};
	vkMapMemory(eng_data.logical_device,
				staging_buff_mem,
				0,
				buff_size,
				0,
				&data);
	memcpy(data, buff, (size_t)buff_size);
	vkUnmapMemory(eng_data.logical_device, staging_buff_mem);

	create_buffer(eng_data,
				  buff_size,
				  VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
				  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				  dst,
				  dst_mem);
	copy_buffer(eng_data, staging_buff, dst, buff_size);

	vkDestroyBuffer(eng_data.logical_device, staging_buff, nullptr);
	vkFreeMemory(eng_data.logical_device, staging_buff_mem, nullptr);
}

void liboceanlight::engine::recreate_swapchain(liboceanlight::window& w,
											   engine_data& eng_data)
{
	int width {0}, height {0};
	glfwGetFramebufferSize(w.window_pointer, &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(w.window_pointer, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(eng_data.logical_device);
	cleanup_swapchain(eng_data);
	get_swapchain_details(w, eng_data);
	create_swapchain(eng_data);
	create_image_views(eng_data);
	create_depth_resources(eng_data);
	create_framebuffers(eng_data);
}
