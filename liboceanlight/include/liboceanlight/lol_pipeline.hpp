#ifndef LIBOCEANLIGHT_PIPELINE_HPP_INCLUDED
#define LIBOCEANLIGHT_PIPELINE_HPP_INCLUDED
#include <vector>
#include <liboceanlight/lol_window.hpp>
#include <vulkan/vulkan_core.h>

namespace liboceanlight::pipeline
{
	using pipeline_data = struct lol_pipeline_data_struct
	{
		VkFormat depth_fmt {VK_FORMAT_D24_UNORM_S8_UINT};
		VkRenderPass render_pass {nullptr};
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
		VkPipelineLayout pipeline_layout {nullptr};
		VkPipeline pipeline {nullptr};
	};
	void create_render_pass(liboceanlight::window&, VkDevice);
	void descriptor_set_layout(VkDevice,
							   std::vector<VkDescriptorSetLayoutBinding>&,
							   VkDescriptorSetLayout&);
	void create_pipeline(VkDevice, VkExtent2D&);
} /* namespace liboceanlight::pipeline */
extern liboceanlight::pipeline::pipeline_data pipe_data;
#endif /* LIBOCEANLIGHT_PIPELINE_HPP_INCLUDED */
