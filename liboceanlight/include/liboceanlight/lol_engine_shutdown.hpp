#ifndef LIBOCEANLIGHT_ENGINE_SHUTDOWN_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_SHUTDOWN_HPP_INCLUDED
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine.hpp>

namespace liboceanlight::engine
{
	void cleanup_debug_messenger(engine_data&);
	void cleanup_instance(engine_data&);
	void cleanup_logical_device(engine_data&);
	void cleanup_surface(engine_data&);
	void cleanup_swapchain(engine_data&);
	void cleanup_vertex_buffer(engine_data&);
	void cleanup_index_buffer(engine_data&);
	void cleanup_uniform_buffers(engine_data&);
	void cleanup_descriptor_pool(engine_data&);
	void cleanup_pipeline(engine_data&);
	void cleanup_commands(engine_data&);
	void cleanup_semaphores(engine_data&);
	void cleanup_fences(engine_data&);
	void deinitialize(engine_data&);
	void shutdown(engine_data&);
} /* namespace liboceanlight::engine */

#endif /* LIBOCEANLIGHT_ENGINE_SHUTDOWN_HPP_INCLUDED */
