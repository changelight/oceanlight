#include <array>
#include <cstdint>
#include <stdexcept>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_pipeline.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_utility.hpp>
#include <liboceanlight/lol_resource.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <vector>

liboceanlight::pipeline::pipeline_data pipe_data;

void liboceanlight::pipeline::create_render_pass(liboceanlight::window& w,
												 VkDevice device)
{
	VkAttachmentDescription color_attachment {};
	color_attachment.format = w.surface_format.format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_ref {};
	color_ref.attachment = 0;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth {};
	depth.format = pipe_data.depth_fmt;
	depth.samples = VK_SAMPLE_COUNT_1_BIT;
	depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_ref {};
	depth_ref.attachment = 1;
	depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_desc {};
	subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_desc.colorAttachmentCount = 1;
	subpass_desc.pColorAttachments = &color_ref;
	subpass_desc.pDepthStencilAttachment = &depth_ref;

	VkSubpassDependency subpass_dep {};
	subpass_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dep.dstSubpass = 0;
	subpass_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
							   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
							   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpass_dep.srcAccessMask = 0;
	subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
								VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array attachments {color_attachment, depth};
	VkRenderPassCreateInfo rp_info {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	rp_info.pAttachments = attachments.data();
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass_desc;
	rp_info.dependencyCount = 1;
	rp_info.pDependencies = &subpass_dep;

	VkResult rv = vkCreateRenderPass(device,
									 &rp_info,
									 nullptr,
									 &pipe_data.render_pass);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}
}

void liboceanlight::pipeline::descriptor_set_layout(
	VkDevice device,
	std::vector<VkDescriptorSetLayoutBinding>& bindings,
	VkDescriptorSetLayout& dst_layout)
{
	VkDescriptorSetLayoutCreateInfo layout_info {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
	layout_info.pBindings = bindings.data();

	VkResult rv = vkCreateDescriptorSetLayout(device,
											  &layout_info,
											  nullptr,
											  &dst_layout);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}
}

void liboceanlight::pipeline::create_pipeline(VkDevice device,
											  VkExtent2D& extent)
{
	const auto vs_code = utility::read_file(SHADER_PATH "vertex_shader.spv");
	const auto fs_code = utility::read_file(SHADER_PATH "fragment_shader.spv");

	VkShaderModule vs = resource::shader(device, vs_code);
	VkShaderModule fs = resource::shader(device, fs_code);

	VkPipelineShaderStageCreateInfo vs_info {};
	vs_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vs_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vs_info.module = vs;
	vs_info.pName = "main";

	VkPipelineShaderStageCreateInfo fs_info {};
	fs_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fs_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fs_info.module = fs;
	fs_info.pName = "main";

	std::array shader_stages {vs_info, fs_info};
	auto binding_desc = engine::vertex::get_binding_desc();
	auto attribute_descs = engine::vertex::get_attribute_descs();

	VkPipelineVertexInputStateCreateInfo vi_info {};
	vi_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi_info.vertexBindingDescriptionCount = 1;
	vi_info.pVertexBindingDescriptions = &binding_desc;
	vi_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(
		attribute_descs.size());
	vi_info.pVertexAttributeDescriptions = attribute_descs.data();

	VkPipelineInputAssemblyStateCreateInfo ia_info {};
	ia_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	ia_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor {};
	scissor.offset = {0, 0};
	scissor.extent = extent;

	static constexpr std::array<VkDynamicState, 2> dyn_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo ds_info {};
	ds_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	ds_info.dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
	ds_info.pDynamicStates = dyn_states.data();

	VkPipelineViewportStateCreateInfo vp_info {};
	vp_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp_info.viewportCount = 1;
	vp_info.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo r_info {};
	r_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	r_info.depthClampEnable = VK_FALSE;
	r_info.rasterizerDiscardEnable = VK_FALSE;
	r_info.polygonMode = VK_POLYGON_MODE_FILL;
	r_info.lineWidth = 1.0f;
	r_info.cullMode = VK_CULL_MODE_BACK_BIT;
	r_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	r_info.depthBiasEnable = VK_FALSE;
	r_info.depthBiasConstantFactor = 0.0f;
	r_info.depthBiasClamp = 0.0f;
	r_info.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo ms_info {};
	ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms_info.sampleShadingEnable = VK_FALSE;
	ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	ms_info.minSampleShading = 1.0f;
	ms_info.pSampleMask = nullptr;
	ms_info.alphaToCoverageEnable = VK_FALSE;
	ms_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo stencil {};
	stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	stencil.depthTestEnable = VK_TRUE;
	stencil.depthWriteEnable = VK_TRUE;
	stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	stencil.depthBoundsTestEnable = VK_FALSE;
	stencil.stencilTestEnable = VK_FALSE;
	stencil.minDepthBounds = 0.0f; // Optional
	stencil.maxDepthBounds = 1.0f; // Optional

	VkPipelineColorBlendAttachmentState color_blend {};
	color_blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
								 VK_COLOR_COMPONENT_G_BIT |
								 VK_COLOR_COMPONENT_B_BIT |
								 VK_COLOR_COMPONENT_A_BIT;
	color_blend.blendEnable = VK_FALSE;
	color_blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo cb_info {};
	cb_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb_info.logicOpEnable = VK_FALSE;
	cb_info.logicOp = VK_LOGIC_OP_COPY;
	cb_info.attachmentCount = 1;
	cb_info.pAttachments = &color_blend;
	cb_info.blendConstants[0] = 0.0f;
	cb_info.blendConstants[1] = 0.0f;
	cb_info.blendConstants[2] = 0.0f;
	cb_info.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(
		pipe_data.descriptor_set_layouts.size());
	pipeline_layout_info.pSetLayouts = pipe_data.descriptor_set_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	auto rv = vkCreatePipelineLayout(dev_data.device,
									 &pipeline_layout_info,
									 nullptr,
									 &pipe_data.pipeline_layout);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	VkGraphicsPipelineCreateInfo pipeline_info {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pVertexInputState = &vi_info;
	pipeline_info.pInputAssemblyState = &ia_info;
	pipeline_info.pViewportState = &vp_info;
	pipeline_info.pRasterizationState = &r_info;
	pipeline_info.pMultisampleState = &ms_info;
	pipeline_info.pDepthStencilState = &stencil;
	pipeline_info.pColorBlendState = &cb_info;
	pipeline_info.pDynamicState = &ds_info;
	pipeline_info.layout = pipe_data.pipeline_layout;
	pipeline_info.renderPass = pipe_data.render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	rv = vkCreateGraphicsPipelines(dev_data.device,
								   VK_NULL_HANDLE,
								   1,
								   &pipeline_info,
								   nullptr,
								   &pipe_data.pipeline);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	vkDestroyShaderModule(device, vs, nullptr);
	vkDestroyShaderModule(device, fs, nullptr);
}
