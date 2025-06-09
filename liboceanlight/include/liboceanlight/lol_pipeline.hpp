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
		VkDescriptorSetLayout descriptor_set_layout {nullptr};
		VkPipelineLayout pipeline_layout {nullptr};
		VkPipeline pipeline {nullptr};
	};
	int create_pipeline(liboceanlight::window&);
	void create_render_pass(liboceanlight::window&, VkDevice);
	void create_descriptor_set_layout(VkDevice);
	void create_pipeline(VkDevice, VkExtent2D&);
} /* namespace liboceanlight::pipeline */
extern liboceanlight::pipeline::pipeline_data pipe_data;
#endif /* LIBOCEANLIGHT_PIPELINE_HPP_INCLUDED */
