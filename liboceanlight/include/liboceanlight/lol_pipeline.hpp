#ifndef LIBOCEANLIGHT_PIPELINE_HPP_INCLUDED
#define LIBOCEANLIGHT_PIPELINE_HPP_INCLUDED
#include <liboceanlight/lol_window.hpp>
#include <vulkan/vulkan_core.h>

namespace liboceanlight::pipeline
{
	using pipeline_data = struct lol_pipeline_data_struct
	{
		VkFormat depth_fmt {VK_FORMAT_D32_SFLOAT};
		VkRenderPass render_pass {nullptr};
	};
	int create_pipeline(liboceanlight::window&);
    void create_render_pass(liboceanlight::window&);
} /* namespace liboceanlight::pipeline */
extern liboceanlight::pipeline::pipeline_data pipe_data;
#endif /* LIBOCEANLIGHT_PIPELINE_HPP_INCLUDED */
