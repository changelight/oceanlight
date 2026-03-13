#ifndef LIBOCEANLIGHT_ENGINE_SHUTDOWN_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_SHUTDOWN_HPP_INCLUDED
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_window.hpp>
#include <vulkan/vulkan_core.h>

namespace liboceanlight::engine
{
	void cleanup_debug_messenger();
	void cleanup_instance(VkInstance&);
	void cleanup_logical_device();
	void cleanup_surface(VkSurfaceKHR&);
	void cleanup_swapchain();
	void cleanup_images();
	void cleanup_vertex_buffer(VkBuffer&, VkDeviceMemory&);
	void cleanup_index_buffer(VkBuffer&, VkDeviceMemory&);
	void cleanup_uniform_buffers();
	void cleanup_descriptor_pool(VkDevice&,
								 VkDescriptorPool&,
								 std::vector<VkDescriptorSetLayout>&);
	void cleanup_pipeline();
	void cleanup_commands(VkCommandPool);
	void cleanup_semaphores();
	void cleanup_fences();
	void deinitialize(liboceanlight::window&);
	void shutdown(liboceanlight::window&);
} /* namespace liboceanlight::engine */

namespace liboceanlight::models
{
	void cleanup_models(std::vector<lol_model>&);
} /* namespace liboceanlight::models */
#endif /* LIBOCEANLIGHT_ENGINE_SHUTDOWN_HPP_INCLUDED */
