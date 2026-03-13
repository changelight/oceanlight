#include <gsl/gsl>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_instance.hpp>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_swapchain.hpp>
#include <liboceanlight/lol_pipeline.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>

void liboceanlight::engine::shutdown(liboceanlight::window& w)
{
	cleanup_fences();
	cleanup_semaphores();
	cleanup_commands(init_data.command_pool);
	cleanup_pipeline();
	cleanup_swapchain();
	cleanup_images();
	cleanup_descriptor_pool(dev_data.device,
							eng_data.descriptor_pool,
							pipe_data.descriptor_set_layouts);
	models::cleanup_models(eng_data.model_list);
	cleanup_uniform_buffers();
	cleanup_surface(w.surface);
	cleanup_logical_device();
	cleanup_debug_messenger();
	cleanup_instance(inst_data.vulkan_instance);
}

void liboceanlight::engine::cleanup_fences()
{
	const size_t n {eng_data.in_flight_fences.size()};
	for (size_t i {0}; i < n; ++i)
	{
		vkDestroyFence(
			dev_data.device,
			gsl::at(eng_data.in_flight_fences, static_cast<long long>(i)),
			nullptr);
	}
}

void liboceanlight::engine::cleanup_semaphores()
{
	const size_t signal_sems_n {eng_data.signal_sems.size()};
	for (size_t i {0}; i < signal_sems_n; ++i)
	{
		vkDestroySemaphore(
			dev_data.device,
			gsl::at(eng_data.signal_sems, static_cast<long long>(i)),
			nullptr);
	}

	const size_t wait_sems_n {eng_data.wait_sems.size()};
	for (size_t i {0}; i < wait_sems_n; ++i)
	{
		vkDestroySemaphore(
			dev_data.device,
			gsl::at(eng_data.wait_sems, static_cast<long long>(i)),
			nullptr);
	}
}

void liboceanlight::engine::cleanup_commands(VkCommandPool command_pool)
{
	if (init_data.command_pool)
	{
		vkDestroyCommandPool(dev_data.device, init_data.command_pool, nullptr);
	}
}

void liboceanlight::engine::cleanup_pipeline()
{
	if (pipe_data.pipeline)
	{
		vkDestroyPipeline(dev_data.device, pipe_data.pipeline, nullptr);
	}

	if (pipe_data.pipeline_layout)
	{
		vkDestroyPipelineLayout(dev_data.device,
								pipe_data.pipeline_layout,
								nullptr);
	}

	if (pipe_data.render_pass)
	{
		vkDestroyRenderPass(dev_data.device, pipe_data.render_pass, nullptr);
	}
}

void liboceanlight::engine::cleanup_swapchain()
{
	vkDestroyImageView(dev_data.device, init_data.depth_img_view, nullptr);
	vkDestroyImage(dev_data.device, init_data.depth_img, nullptr);
	vkFreeMemory(dev_data.device, init_data.depth_img_mem, nullptr);

	const std::vector<int>::size_type fb_n = init_data.frame_buffers.size();
	for (std::vector<int>::size_type i {0}; i < fb_n; ++i)
	{
		vkDestroyFramebuffer(dev_data.device,
							 init_data.frame_buffers[i],
							 nullptr);
	}

	const std::vector<int>::size_type iv_n {swap_data.image_views.size()};
	for (std::vector<int>::size_type i {0}; i < iv_n; ++i)
	{
		vkDestroyImageView(dev_data.device, swap_data.image_views[i], nullptr);
	}

	if (swap_data.swap_chain)
	{
		vkDestroySwapchainKHR(dev_data.device, swap_data.swap_chain, nullptr);
	}
}

void liboceanlight::engine::cleanup_images()
{
	for (const auto& model : eng_data.model_list)
	{
		vkDestroySampler(dev_data.device, model.texture.sampler, nullptr);
		vkDestroyImageView(dev_data.device, model.texture.img_view, nullptr);
		vkDestroyImage(dev_data.device, model.texture.img, nullptr);
		vkFreeMemory(dev_data.device, model.texture.img_mem, nullptr);
	}
}

void liboceanlight::engine::cleanup_descriptor_pool(
	VkDevice& dev,
	VkDescriptorPool& pool,
	std::vector<VkDescriptorSetLayout>& layouts)
{
	if (pool)
	{
		vkDestroyDescriptorPool(dev, pool, nullptr);
	}

	for (const auto& layout : layouts)
	{
		if (layout)
		{
			vkDestroyDescriptorSetLayout(dev, layout, nullptr);
		}
	}
}

void liboceanlight::engine::cleanup_uniform_buffers()
{
	if (!eng_data.uniform_buffers.empty())
	{
		for (size_t i {0}; i < engine::max_frames_in_flight; ++i)
		{
			vkDestroyBuffer(
				dev_data.device,
				gsl::at(eng_data.uniform_buffers, static_cast<long long>(i)),
				nullptr);

			vkFreeMemory(dev_data.device,
						 gsl::at(eng_data.uniform_buffers_mem,
								 static_cast<long long>(i)),
						 nullptr);
		}
	}

	for (const auto& model : eng_data.model_list)
	{
		for (auto i {0}; i < engine::max_frames_in_flight; ++i)
		{
			if (model.uniforms_mapped[i])
			{
				vkFreeMemory(dev_data.device, model.uniforms_mem[i], nullptr);
			}

			if (model.uniforms[i])
			{
				vkDestroyBuffer(dev_data.device, model.uniforms[i], nullptr);
			}
		}
	}
}

void cleanup_vertex_buffer(VkBuffer& vertex_buffer,
						   VkDeviceMemory& vertex_buffer_mem)
{
	if (vertex_buffer)
	{
		vkDestroyBuffer(dev_data.device, vertex_buffer, nullptr);
	}

	if (vertex_buffer_mem)
	{
		vkFreeMemory(dev_data.device, vertex_buffer_mem, nullptr);
	}
}

void cleanup_index_buffer(VkBuffer& index_buffer,
						  VkDeviceMemory& index_buffer_mem)
{
	if (index_buffer)
	{
		vkDestroyBuffer(dev_data.device, index_buffer, nullptr);
	}

	if (index_buffer_mem)
	{
		vkFreeMemory(dev_data.device, index_buffer_mem, nullptr);
	}
}

void liboceanlight::models::cleanup_models(
	std::vector<liboceanlight::models::lol_model>& models)
{
	for (int i {0}; i < models.size(); ++i)
	{
		cleanup_vertex_buffer(models[i].vertex_buffer,
							  models[i].vertex_buffer_mem);

		cleanup_index_buffer(models[i].index_buffer,
							 models[i].index_buffer_mem);
	}
}

void liboceanlight::engine::cleanup_surface(VkSurfaceKHR& surface)
{
	if (surface)
	{
		vkDestroySurfaceKHR(inst_data.vulkan_instance, surface, nullptr);
	}
}

void liboceanlight::engine::cleanup_logical_device()
{
	if (dev_data.device)
	{
		vkDestroyDevice(dev_data.device, nullptr);
	}
}

void liboceanlight::engine::cleanup_debug_messenger()
{
	if (inst_data.dbg_messenger)
	{
		DestroyDebugUtilsMessengerEXT(inst_data.vulkan_instance,
									  inst_data.dbg_messenger,
									  nullptr);
	}
}

void liboceanlight::engine::cleanup_instance(VkInstance& instance)
{
	if (instance)
	{
		vkDestroyInstance(instance, nullptr);
	}
}
