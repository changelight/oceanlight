#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_utility.hpp>
#include <config.h>

using namespace liboceanlight::engine;

void liboceanlight::engine::start(liboceanlight::window& window,
								  engine_data& eng_data)
{
	init(window, eng_data);
	run(window, eng_data);
}

void liboceanlight::engine::run(liboceanlight::window& window,
								engine_data& eng_data)
{
	while (!window.should_close())
	{
		glfwWaitEvents();
		draw_frame(window, eng_data);
	}

	vkDeviceWaitIdle(eng_data.logical_device);
}

void liboceanlight::engine::draw_frame(liboceanlight::window& window,
									   engine_data& eng_data)
{
	vkWaitForFences(eng_data.logical_device,
					1,
					&eng_data.in_flight_fences[eng_data.current_frame],
					VK_TRUE,
					UINT64_MAX);

	uint32_t image_index;
	VkResult rv;
	rv = vkAcquireNextImageKHR(eng_data.logical_device,
							   eng_data.swap_chain,
							   UINT64_MAX,
							   eng_data.wait_sems[eng_data.current_frame],
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
				  &eng_data.in_flight_fences[eng_data.current_frame]);

	vkResetCommandBuffer(eng_data.command_buffers[eng_data.current_frame], 0);
	record_cmd_buffer(eng_data,
					  eng_data.command_buffers[eng_data.current_frame],
					  image_index);

	VkSubmitInfo submit_info {};
	VkSemaphore signal_sems[] {eng_data.signal_sems[eng_data.current_frame]};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_sems;

	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore wait_sems[] {eng_data.wait_sems[eng_data.current_frame]};
	VkPipelineStageFlags wait_stages[] {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_sems;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers =
		&eng_data.command_buffers[eng_data.current_frame];

	rv = vkQueueSubmit(eng_data.graphics_queue,
					   1,
					   &submit_info,
					   eng_data.in_flight_fences[eng_data.current_frame]);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}

	VkPresentInfoKHR present_info {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_sems;

	VkSwapchainKHR swap_chains[] {eng_data.swap_chain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
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

	VkResult rv;
	rv = vkBeginCommandBuffer(cmd_buffer, &begin_info);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer");
	}

	VkRenderPassBeginInfo pass_info {};
	pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	pass_info.renderPass = eng_data.render_pass;
	pass_info.framebuffer = eng_data.frame_buffers[image_index];
	pass_info.renderArea.offset = {0, 0};
	pass_info.renderArea.extent = eng_data.swap_extent;
	VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	pass_info.clearValueCount = 1;
	pass_info.pClearValues = &clear_color;

	vkCmdBeginRenderPass(cmd_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(cmd_buffer,
					  VK_PIPELINE_BIND_POINT_GRAPHICS,
					  eng_data.graphics_pipeline);

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

	vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(cmd_buffer);

	rv = vkEndCommandBuffer(cmd_buffer);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer");
	}
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
	create_framebuffers(eng_data);
}
