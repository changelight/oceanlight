#include <vulkan/vulkan.h>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>

using namespace liboceanlight::engine;

void liboceanlight::engine::shutdown(engine_data& eng_data)
{
	deinitialize(eng_data);
}

void liboceanlight::engine::deinitialize(engine_data& eng_data)
{
	cleanup_fences(eng_data);
	cleanup_semaphores(eng_data);
	cleanup_commands(eng_data);
	cleanup_pipeline(eng_data);
	cleanup_swapchain(eng_data);
	cleanup_surface(eng_data);
	cleanup_logical_device(eng_data);
	cleanup_debug_messenger(eng_data);
	cleanup_instance(eng_data);
}

void liboceanlight::engine::cleanup_fences(engine_data& eng_data)
{
	const size_t n {eng_data.in_flight_fences.size()};
	for (size_t i {0}; i < n; ++i)
	{
		vkDestroyFence(eng_data.logical_device,
					   eng_data.in_flight_fences[i],
					   nullptr);
	}
}

void liboceanlight::engine::cleanup_semaphores(engine_data& eng_data)
{
	const size_t signal_sems_n {eng_data.signal_sems.size()};
	for (size_t i {0}; i < signal_sems_n; ++i)
	{
		vkDestroySemaphore(eng_data.logical_device,
						   eng_data.signal_sems[i],
						   nullptr);
	}

	const size_t wait_sems_n {eng_data.wait_sems.size()};
	for (size_t i {0}; i < wait_sems_n; ++i)
	{
		vkDestroySemaphore(eng_data.logical_device,
						   eng_data.wait_sems[i],
						   nullptr);
	}
}

void liboceanlight::engine::cleanup_commands(engine_data& eng_data)
{
	if (eng_data.command_pool)
	{
		vkDestroyCommandPool(eng_data.logical_device,
							 eng_data.command_pool,
							 nullptr);
	}
}

void liboceanlight::engine::cleanup_pipeline(engine_data& eng_data)
{
	if (eng_data.graphics_pipeline)
	{
		vkDestroyPipeline(eng_data.logical_device,
						  eng_data.graphics_pipeline,
						  nullptr);
	}

	if (eng_data.pipeline_layout)
	{
		vkDestroyPipelineLayout(eng_data.logical_device,
								eng_data.pipeline_layout,
								nullptr);
	}

	if (eng_data.render_pass)
	{
		vkDestroyRenderPass(eng_data.logical_device,
							eng_data.render_pass,
							nullptr);
	}
}

void liboceanlight::engine::cleanup_swapchain(engine_data& eng_data)
{
	const std::vector<int>::size_type fb_n = eng_data.frame_buffers.size();
	for (std::vector<int>::size_type i {0}; i < fb_n; ++i)
	{
		vkDestroyFramebuffer(eng_data.logical_device,
							 eng_data.frame_buffers[i],
							 nullptr);
	}

	const std::vector<int>::size_type iv_n = eng_data.image_views.size();
	for (std::vector<int>::size_type i {0}; i < iv_n; ++i)
	{
		vkDestroyImageView(eng_data.logical_device,
						   eng_data.image_views[i],
						   nullptr);
	}

	if (eng_data.swap_chain)
	{
		vkDestroySwapchainKHR(eng_data.logical_device,
							  eng_data.swap_chain,
							  nullptr);
	}
}

void liboceanlight::engine::cleanup_surface(engine_data& eng_data)
{
	if (eng_data.window_surface)
	{
		vkDestroySurfaceKHR(eng_data.vulkan_instance,
							eng_data.window_surface,
							nullptr);
	}
}

void liboceanlight::engine::cleanup_logical_device(engine_data& eng_data)
{
	if (eng_data.logical_device)
	{
		vkDestroyDevice(eng_data.logical_device, nullptr);
	}
}

void liboceanlight::engine::cleanup_debug_messenger(engine_data& eng_data)
{
	if (eng_data.dbg_messenger)
	{
		DestroyDebugUtilsMessengerEXT(eng_data.vulkan_instance,
									  eng_data.dbg_messenger,
									  nullptr);
	}
}

void liboceanlight::engine::cleanup_instance(engine_data& eng_data)
{
	if (eng_data.vulkan_instance)
	{
		vkDestroyInstance(eng_data.vulkan_instance, nullptr);
	}
}
