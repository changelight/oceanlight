#include <algorithm>
#include <array>
#include <config.h>
#include <cstring>
#include <filesystem>
#include <gsl/gsl>
#include <iostream>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>
#include <liboceanlight/lol_utility.hpp>
#include <liboceanlight/lol_device.hpp>
#include <stb_image.h>
#include <stdexcept>
#include <tiny_gltf.h>
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <vector>
#include <liboceanlight/lol_instance.hpp>

namespace fs = std::filesystem;
using namespace liboceanlight::engine;

int liboceanlight::engine::init(liboceanlight::window& w,
								engine_data& eng_data)
{
	create_instance_new();
	eng_data.vulkan_instance = inst_data.vulkan_instance;
	eng_data.dbg_messenger = inst_data.dbg_messenger;
	eng_data.validation_layer_enabled = inst_data.validation_layer_enabled;
	//create_instance(eng_data);
	create_physical_device_new(inst_data.vulkan_instance);
	eng_data.physical_device = dev_data.physical_device;
	//create_physical_device(eng_data);

	w.create_surface(inst_data.vulkan_instance);
	eng_data.window_surface = w.surface;
	//create_surface(w, eng_data);
	check_device_queue_support(w.surface);
	eng_data.graphics_queue_index = dev_data.graphics_queue_index;
	//get_queue_fams(eng_data);
	create_logical_device(eng_data);

	get_swapchain_details(w, eng_data);
	create_swapchain(eng_data);
	create_image_views(eng_data);

	create_render_pass(eng_data);
	create_descriptor_set_layout(eng_data);
	create_pipeline(eng_data);

	create_cmd_pool(eng_data);
	create_depth_resources(eng_data);
	create_framebuffers(eng_data);
	create_texture_img(eng_data);
	create_texture_img_view(eng_data);
	create_texture_sampler(eng_data);

	load_models(eng_data);
	create_vertex_buffers(eng_data);
	create_index_buffers(eng_data);
	create_uniform_buffers(eng_data);
	create_descriptor_pool(eng_data);
	create_descriptor_sets(eng_data);
	create_cmd_buffer(eng_data);
	create_sync_objects(eng_data);

	return 1;
}

void liboceanlight::engine::create_logical_device(engine_data& eng_data)
{
	float queue_priority {1.0f};
	VkDeviceQueueCreateInfo queue_info {};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = eng_data.graphics_queue_index;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures requested_dev_features {
		.samplerAnisotropy = VK_TRUE};
	VkDeviceCreateInfo dev_info {};
	dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_info.queueCreateInfoCount = 1;
	dev_info.pQueueCreateInfos = &queue_info;
	dev_info.pEnabledFeatures = &requested_dev_features;
	dev_info.enabledExtensionCount = static_cast<uint32_t>(
		eng_data.dev_extensions.size());
	dev_info.ppEnabledExtensionNames = eng_data.dev_extensions.data();

	VkResult rv = vkCreateDevice(eng_data.physical_device,
								 &dev_info,
								 nullptr,
								 &eng_data.logical_device);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}

	vkGetDeviceQueue(eng_data.logical_device,
					 eng_data.graphics_queue_index,
					 0,
					 &eng_data.graphics_queue);
}

void liboceanlight::engine::get_swapchain_details(
	liboceanlight::window& window,
	engine_data& eng_data)
{
	VkResult rv = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		eng_data.physical_device,
		eng_data.window_surface,
		&eng_data.capabilities);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get surface capabilities");
	}

	if (eng_data.capabilities.currentExtent.width !=
		std::numeric_limits<uint32_t>::max())
	{
		eng_data.swap_extent = eng_data.capabilities.currentExtent;
	}
	else
	{
		int width {}, height {};
		glfwGetFramebufferSize(window.window_pointer, &width, &height);

		VkExtent2D actual_extent {static_cast<uint32_t>(width),
								  static_cast<uint32_t>(height)};

		actual_extent.width = std::clamp(
			actual_extent.width,
			eng_data.capabilities.minImageExtent.width,
			eng_data.capabilities.maxImageExtent.width);

		actual_extent.height = std::clamp(
			actual_extent.height,
			eng_data.capabilities.minImageExtent.height,
			eng_data.capabilities.maxImageExtent.height);

		eng_data.swap_extent = actual_extent;
	}

	uint32_t count {};
	vkGetPhysicalDeviceSurfaceFormatsKHR(eng_data.physical_device,
										 eng_data.window_surface,
										 &count,
										 nullptr);

	if (count == 0)
	{
		throw std::runtime_error("No surface formats found");
	}

	std::vector<VkSurfaceFormatKHR> surface_formats(count);
	rv = vkGetPhysicalDeviceSurfaceFormatsKHR(eng_data.physical_device,
											  eng_data.window_surface,
											  &count,
											  surface_formats.data());

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get surface formats");
	}

	eng_data.surface_format = surface_formats[0];
	for (const auto& available_format : surface_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			eng_data.surface_format = available_format;
			break;
		}
	}

	count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(eng_data.physical_device,
											  eng_data.window_surface,
											  &count,
											  nullptr);

	if (count == 0)
	{
		throw std::runtime_error("No surface present modes found");
	}

	std::vector<VkPresentModeKHR> present_modes(count);
	rv = vkGetPhysicalDeviceSurfacePresentModesKHR(eng_data.physical_device,
												   eng_data.window_surface,
												   &count,
												   present_modes.data());

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to get surface present modes");
	}

	eng_data.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& available_present_mode : present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			eng_data.present_mode = available_present_mode;
			break;
		}
	}
}

void liboceanlight::engine::create_swapchain(engine_data& eng_data)
{
	uint32_t img_count = eng_data.capabilities.minImageCount + 1;
	uint32_t max_img_count = eng_data.capabilities.maxImageCount;

	if (max_img_count > 0)
	{
		img_count = std::max(img_count, max_img_count);
	}
	else
	{
		throw std::runtime_error("Error: Max swapchain image count is 0.");
	}

	VkSwapchainCreateInfoKHR c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	c_info.surface = eng_data.window_surface;
	c_info.minImageCount = img_count;
	c_info.imageFormat = eng_data.surface_format.format;
	c_info.imageColorSpace = eng_data.surface_format.colorSpace;
	c_info.imageExtent = eng_data.swap_extent;
	c_info.imageArrayLayers = 1;
	c_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	c_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	c_info.queueFamilyIndexCount = 0;
	c_info.pQueueFamilyIndices = nullptr;
	c_info.preTransform = eng_data.capabilities.currentTransform;
	c_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	c_info.presentMode = eng_data.present_mode;
	c_info.clipped = VK_TRUE;
	c_info.oldSwapchain = VK_NULL_HANDLE;

	VkResult rv = vkCreateSwapchainKHR(eng_data.logical_device,
									   &c_info,
									   nullptr,
									   &eng_data.swap_chain);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain");
	}

	img_count = 0;
	vkGetSwapchainImagesKHR(eng_data.logical_device,
							eng_data.swap_chain,
							&img_count,
							nullptr);

	eng_data.images.resize(img_count);
	vkGetSwapchainImagesKHR(eng_data.logical_device,
							eng_data.swap_chain,
							&img_count,
							eng_data.images.data());
}

void liboceanlight::engine::create_image_views(engine_data& eng_data)
{
	const std::vector<int>::size_type n = eng_data.images.size();
	eng_data.image_views.resize(n);
	for (std::vector<int>::size_type i {0}; i < n; ++i)
	{
		eng_data.image_views[i] = create_image_view(
			eng_data,
			eng_data.images[i],
			eng_data.surface_format.format,
			VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

VkImageView liboceanlight::engine::create_image_view(
	engine_data& eng_data,
	VkImage img,
	VkFormat fmt,
	VkImageAspectFlags aspect_flags)
{
	VkImageViewCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	c_info.image = img;
	c_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	c_info.format = fmt;
	c_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	c_info.subresourceRange.aspectMask = aspect_flags;
	c_info.subresourceRange.baseMipLevel = 0;
	c_info.subresourceRange.levelCount = 1;
	c_info.subresourceRange.baseArrayLayer = 0;
	c_info.subresourceRange.layerCount = 1;

	VkImageView img_view {};
	VkResult rv = vkCreateImageView(eng_data.logical_device,
									&c_info,
									nullptr,
									&img_view);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image view");
	}

	return img_view;
}

void liboceanlight::engine::create_render_pass(engine_data& eng_data)
{
	VkAttachmentDescription color_attachment {};
	color_attachment.format = eng_data.surface_format.format;
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

	VkAttachmentDescription depth_attachment {};
	depth_attachment.format = eng_data.depth_fmt;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_desc {};
	subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_desc.colorAttachmentCount = 1;
	subpass_desc.pColorAttachments = &color_ref;
	subpass_desc.pDepthStencilAttachment = &depth_attachment_ref;

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

	std::array attachments {color_attachment, depth_attachment};
	VkRenderPassCreateInfo render_pass_info {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = static_cast<uint32_t>(
		attachments.size());
	render_pass_info.pAttachments = attachments.data();
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_desc;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &subpass_dep;

	VkResult rv = vkCreateRenderPass(eng_data.logical_device,
									 &render_pass_info,
									 nullptr,
									 &eng_data.render_pass);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}
}

void liboceanlight::engine::create_descriptor_set_layout(engine_data& eng_data)
{
	VkDescriptorSetLayoutBinding ubo_layout_binding {};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding sampler_layout_binding {};
	sampler_layout_binding.binding = 1;
	sampler_layout_binding.descriptorCount = 1;
	sampler_layout_binding.descriptorType =
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_binding.pImmutableSamplers = nullptr;
	sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array bindings {ubo_layout_binding, sampler_layout_binding};
	VkDescriptorSetLayoutCreateInfo layout_info {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
	layout_info.pBindings = bindings.data();

	VkResult rv = vkCreateDescriptorSetLayout(eng_data.logical_device,
											  &layout_info,
											  nullptr,
											  &eng_data.descriptor_set_layout);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout");
	}
}

void liboceanlight::engine::create_pipeline(engine_data& eng_data)
{
	auto vs_code = read_file(SHADER_PATH "vertex_shader.spv");
	auto fs_code = read_file(SHADER_PATH "fragment_shader.spv");

	VkShaderModule vs = create_shader(eng_data, vs_code);
	VkShaderModule fs = create_shader(eng_data, fs_code);

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
	auto binding_desc = vertex::get_binding_desc();
	auto attribute_descs = vertex::get_attribute_descs();

	VkPipelineVertexInputStateCreateInfo vertex_input_info {};
	vertex_input_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &binding_desc;
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(
		attribute_descs.size());
	vertex_input_info.pVertexAttributeDescriptions = attribute_descs.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info {};
	input_assembly_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)eng_data.swap_extent.width;
	viewport.height = (float)eng_data.swap_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor {};
	scissor.offset = {0, 0};
	scissor.extent = eng_data.swap_extent;

	std::vector<VkDynamicState> dyn_states = {VK_DYNAMIC_STATE_VIEWPORT,
											  VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dyn_info {};
	dyn_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn_info.dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
	dyn_info.pDynamicStates = dyn_states.data();

	VkPipelineViewportStateCreateInfo viewport_info {};
	viewport_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer_info {};
	rasterizer_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_info.depthClampEnable = VK_FALSE;
	rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer_info.depthBiasEnable = VK_FALSE;
	rasterizer_info.depthBiasConstantFactor = 0.0f;
	rasterizer_info.depthBiasClamp = 0.0f;
	rasterizer_info.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo ms_info {};
	ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms_info.sampleShadingEnable = VK_FALSE;
	ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	ms_info.minSampleShading = 1.0f;
	ms_info.pSampleMask = nullptr;
	ms_info.alphaToCoverageEnable = VK_FALSE;
	ms_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil {};
	depth_stencil.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.minDepthBounds = 0.0f; // Optional
	depth_stencil.maxDepthBounds = 1.0f; // Optional

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

	VkPipelineColorBlendStateCreateInfo color_blend_info {};
	color_blend_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_info.logicOpEnable = VK_FALSE;
	color_blend_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_info.attachmentCount = 1;
	color_blend_info.pAttachments = &color_blend;
	color_blend_info.blendConstants[0] = 0.0f;
	color_blend_info.blendConstants[1] = 0.0f;
	color_blend_info.blendConstants[2] = 0.0f;
	color_blend_info.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &eng_data.descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;

	auto rv = vkCreatePipelineLayout(eng_data.logical_device,
									 &pipeline_layout_info,
									 nullptr,
									 &eng_data.pipeline_layout);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	VkGraphicsPipelineCreateInfo pipeline_info {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages.data();
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &rasterizer_info;
	pipeline_info.pMultisampleState = &ms_info;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blend_info;
	pipeline_info.pDynamicState = &dyn_info;
	pipeline_info.layout = eng_data.pipeline_layout;
	pipeline_info.renderPass = eng_data.render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	rv = vkCreateGraphicsPipelines(eng_data.logical_device,
								   VK_NULL_HANDLE,
								   1,
								   &pipeline_info,
								   nullptr,
								   &eng_data.graphics_pipeline);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	vkDestroyShaderModule(eng_data.logical_device, vs, nullptr);
	vkDestroyShaderModule(eng_data.logical_device, fs, nullptr);
}

void liboceanlight::engine::create_framebuffers(engine_data& eng_data)
{
	size_t n {eng_data.image_views.size()};
	eng_data.frame_buffers.resize(n);

	VkFramebufferCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	c_info.renderPass = eng_data.render_pass;
	c_info.width = eng_data.swap_extent.width;
	c_info.height = eng_data.swap_extent.height;
	c_info.layers = 1;

	VkResult rv {};
	for (size_t i {0}; i < n; ++i)
	{
		std::array attachments {eng_data.image_views[i],
								eng_data.depth_img_view};
		c_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		c_info.pAttachments = attachments.data();

		rv = vkCreateFramebuffer(eng_data.logical_device,
								 &c_info,
								 nullptr,
								 &eng_data.frame_buffers[i]);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffers");
		}
	}
}

void liboceanlight::engine::create_cmd_pool(engine_data& eng_data)
{
	VkCommandPoolCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	c_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	c_info.queueFamilyIndex = eng_data.graphics_queue_index;

	VkResult rv = vkCreateCommandPool(eng_data.logical_device,
									  &c_info,
									  nullptr,
									  &eng_data.command_pool);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}
}

void liboceanlight::engine::create_depth_resources(engine_data& eng_data)
{
	create_image(eng_data,
				 eng_data.swap_extent.width,
				 eng_data.swap_extent.height,
				 eng_data.depth_fmt,
				 VK_IMAGE_TILING_OPTIMAL,
				 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 eng_data.depth_img,
				 eng_data.depth_img_mem);

	eng_data.depth_img_view = create_image_view(eng_data,
												eng_data.depth_img,
												eng_data.depth_fmt,
												VK_IMAGE_ASPECT_DEPTH_BIT);

	transition_img_layout(eng_data,
						  eng_data.depth_img,
						  eng_data.depth_fmt,
						  VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void liboceanlight::engine::create_texture_img(engine_data& eng_data)
{
	int width {}, height {}, channels {}, bytes_per_pixel {STBI_rgb_alpha};
	const char* path {TEXTURE_PATH "viking_room.png"};

	stbi_uc* pixels {nullptr};
	pixels = stbi_load(path, &width, &height, &channels, bytes_per_pixel);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image");
	}

	VkDeviceSize img_size {
		static_cast<VkDeviceSize>(width * height * bytes_per_pixel)};
	VkBuffer staging_buff {nullptr};
	VkDeviceMemory staging_buff_mem {nullptr};

	create_buffer(eng_data,
				  img_size,
				  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				  staging_buff,
				  staging_buff_mem);

	void* data {nullptr};
	vkMapMemory(eng_data.logical_device,
				staging_buff_mem,
				0,
				img_size,
				0,
				&data);
	memcpy(data, pixels, static_cast<size_t>(img_size));
	vkUnmapMemory(eng_data.logical_device, staging_buff_mem);
	stbi_image_free(pixels);

	create_image(eng_data,
				 width,
				 height,
				 VK_FORMAT_R8G8B8A8_SRGB,
				 VK_IMAGE_TILING_OPTIMAL,
				 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 eng_data.texture_img,
				 eng_data.texture_img_mem);

	transition_img_layout(eng_data,
						  eng_data.texture_img,
						  VK_FORMAT_R8G8B8A8_SRGB,
						  VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copy_buffer_to_img(eng_data,
					   staging_buff,
					   eng_data.texture_img,
					   static_cast<uint32_t>(width),
					   static_cast<uint32_t>(height));

	transition_img_layout(eng_data,
						  eng_data.texture_img,
						  VK_FORMAT_R8G8B8A8_SRGB,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(eng_data.logical_device, staging_buff, nullptr);
	vkFreeMemory(eng_data.logical_device, staging_buff_mem, nullptr);
}

void liboceanlight::engine::create_image(engine_data& eng_data,
										 uint32_t width,
										 uint32_t height,
										 VkFormat fmt,
										 VkImageTiling tiling,
										 VkImageUsageFlags usage,
										 VkMemoryPropertyFlags props,
										 VkImage& image,
										 VkDeviceMemory& image_mem)
{
	VkImageCreateInfo image_info {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(width);
	image_info.extent.height = static_cast<uint32_t>(height);
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = fmt;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.flags = 0;

	VkResult rv {};
	rv = vkCreateImage(eng_data.logical_device, &image_info, nullptr, &image);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image");
	}

	VkMemoryRequirements mem_reqs {};
	vkGetImageMemoryRequirements(eng_data.logical_device, image, &mem_reqs);

	VkMemoryAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = find_mem_type(eng_data,
											   mem_reqs.memoryTypeBits,
											   props);

	rv = vkAllocateMemory(eng_data.logical_device,
						  &alloc_info,
						  nullptr,
						  &image_mem);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate image memory");
	}

	vkBindImageMemory(eng_data.logical_device, image, image_mem, 0);
}

VkCommandBuffer liboceanlight::engine::begin_single_time_cmds(
	engine_data& eng_data)
{
	VkCommandBufferAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = eng_data.command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer cmd_buffer {};
	vkAllocateCommandBuffers(eng_data.logical_device,
							 &alloc_info,
							 &cmd_buffer);

	VkCommandBufferBeginInfo begin_info {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd_buffer, &begin_info);
	return cmd_buffer;
}

void liboceanlight::engine::end_single_time_cmds(engine_data& eng_data,
												 VkCommandBuffer& cmd_buffer)
{
	vkEndCommandBuffer(cmd_buffer);

	VkSubmitInfo submit_info {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer;

	vkQueueSubmit(eng_data.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(eng_data.graphics_queue);
	vkFreeCommandBuffers(eng_data.logical_device,
						 eng_data.command_pool,
						 1,
						 &cmd_buffer);
}

bool has_stencil_component(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		   format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void liboceanlight::engine::transition_img_layout(engine_data& eng_data,
												  VkImage img,
												  VkFormat fmt,
												  VkImageLayout old_layout,
												  VkImageLayout new_layout)
{
	VkCommandBuffer cmd_buffer {begin_single_time_cmds(eng_data)};
	VkImageMemoryBarrier barrier {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = img;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	VkPipelineStageFlags src_stage {}, dst_stage {};
	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
		new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			 new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
			 new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
								VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::runtime_error("Unsupported layout transition");
	}

	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (has_stencil_component(fmt))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(cmd_buffer,
						 src_stage,
						 dst_stage,
						 0,
						 0,
						 nullptr,
						 0,
						 nullptr,
						 1,
						 &barrier);

	end_single_time_cmds(eng_data, cmd_buffer);
}

void liboceanlight::engine::copy_buffer_to_img(engine_data& eng_data,
											   VkBuffer buff,
											   VkImage img,
											   uint32_t width,
											   uint32_t height)
{
	VkCommandBuffer cmd_buffer {begin_single_time_cmds(eng_data)};
	VkBufferImageCopy region {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};
	vkCmdCopyBufferToImage(cmd_buffer,
						   buff,
						   img,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						   1,
						   &region);
	end_single_time_cmds(eng_data, cmd_buffer);
}

void liboceanlight::engine::create_texture_img_view(engine_data& eng_data)
{
	eng_data.texture_img_view = create_image_view(eng_data,
												  eng_data.texture_img,
												  VK_FORMAT_R8G8B8A8_SRGB,
												  VK_IMAGE_ASPECT_COLOR_BIT);
}

void liboceanlight::engine::create_texture_sampler(engine_data& eng_data)
{
	VkSamplerCreateInfo c_info {};
	c_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	c_info.magFilter = VK_FILTER_LINEAR;
	c_info.minFilter = VK_FILTER_LINEAR;
	c_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	c_info.anisotropyEnable = VK_TRUE;
	c_info.maxAnisotropy = dev_data.device_props.limits.maxSamplerAnisotropy;
	c_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	c_info.unnormalizedCoordinates = VK_FALSE;
	c_info.compareEnable = VK_FALSE;
	c_info.compareOp = VK_COMPARE_OP_ALWAYS;
	c_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	c_info.mipLodBias = 0.0f;
	c_info.minLod = 0.0f;
	c_info.maxLod = 0.0f;

	VkResult rv = vkCreateSampler(eng_data.logical_device,
								  &c_info,
								  nullptr,
								  &eng_data.texture_sampler);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler");
	}
}

void liboceanlight::engine::load_models(engine_data& eng_data)
{
	for (const auto& file : fs::directory_iterator(MODEL_PATH))
	{
		eng_data.model_list.emplace_back(file.path().filename().string());
		std::cout << "Loaded model " << file.path().filename() << "\n";
	}

	for (auto& model : eng_data.model_list)
	{
		std::string model_file {MODEL_PATH + model.name};
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		auto rv = tinyobj::LoadObj(&attrib,
								   &shapes,
								   &materials,
								   &warn,
								   &err,
								   model_file.c_str());

		if (!rv)
		{
			throw std::runtime_error("Failed to load model " + model_file +
									 "\n" + warn + err);
		}

		std::unordered_map<vertex, uint32_t> unique_vertices {};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				vertex vertex {};

				vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
							  attrib.vertices[3 * index.vertex_index + 1],
							  attrib.vertices[3 * index.vertex_index + 2]};

				vertex.texcoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

				vertex.color = {1.0f, 1.0f, 1.0f};

				if (unique_vertices.count(vertex) == 0)
				{
					unique_vertices[vertex] = static_cast<uint32_t>(
						model.vertices.size());
					model.vertices.push_back(vertex);
				}

				model.indices.push_back(unique_vertices[vertex]);
			}
		}
	}
}

void liboceanlight::engine::create_vertex_buffers(engine_data& eng_data)
{
	for (auto& model : eng_data.model_list)
	{
		upload_buffer(eng_data,
					  model.vertices.data(),
					  sizeof(model.vertices[0]) * model.vertices.size(),
					  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					  model.vertex_buffer,
					  model.vertex_buffer_mem);
	}
}

void liboceanlight::engine::create_index_buffers(engine_data& eng_data)
{
	for (auto& model : eng_data.model_list)
	{
		upload_buffer(eng_data,
					  model.indices.data(),
					  sizeof(model.indices[0]) * model.indices.size(),
					  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					  model.index_buffer,
					  model.index_buffer_mem);
	}
}

void liboceanlight::engine::create_uniform_buffers(engine_data& eng_data)
{
	VkDeviceSize buff_size {sizeof(uniform_buffer_object)};

	for (auto i {0}; i < eng_data.max_frames_in_flight; ++i)
	{
		create_buffer(eng_data,
					  buff_size,
					  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					  gsl::at(eng_data.uniform_buffers, i),
					  gsl::at(eng_data.uniform_buffers_mem, i));

		vkMapMemory(eng_data.logical_device,
					gsl::at(eng_data.uniform_buffers_mem, i),
					0,
					buff_size,
					0,
					&gsl::at(eng_data.uniform_buffers_mapped, i));
	}
}

void liboceanlight::engine::create_descriptor_pool(engine_data& eng_data)
{
	std::array<VkDescriptorPoolSize, 2> pool_sizes {};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(
		eng_data.max_frames_in_flight);
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(
		eng_data.max_frames_in_flight);

	VkDescriptorPoolCreateInfo pool_info {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();
	pool_info.maxSets = static_cast<uint32_t>(eng_data.max_frames_in_flight);

	VkResult rv = vkCreateDescriptorPool(eng_data.logical_device,
										 &pool_info,
										 nullptr,
										 &eng_data.descriptor_pool);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

void liboceanlight::engine::create_descriptor_sets(engine_data& eng_data)
{
	std::vector<VkDescriptorSetLayout> layouts(eng_data.max_frames_in_flight,
											   eng_data.descriptor_set_layout);

	VkDescriptorSetAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = eng_data.descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(
		eng_data.max_frames_in_flight);
	alloc_info.pSetLayouts = layouts.data();

	VkResult rv = vkAllocateDescriptorSets(eng_data.logical_device,
										   &alloc_info,
										   eng_data.descriptor_sets.data());

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	for (int i {0}; i < eng_data.max_frames_in_flight; ++i)
	{
		VkDescriptorBufferInfo buff_info {};
		buff_info.buffer = gsl::at(eng_data.uniform_buffers, i);
		buff_info.offset = 0;
		buff_info.range = sizeof(uniform_buffer_object);

		VkDescriptorImageInfo image_info {};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = eng_data.texture_img_view;
		image_info.sampler = eng_data.texture_sampler;

		std::array<VkWriteDescriptorSet, 2> descriptor_writes {};
		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = gsl::at(eng_data.descriptor_sets, i);
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType =
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pBufferInfo = &buff_info;

		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[1].dstSet = gsl::at(eng_data.descriptor_sets, i);
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].dstArrayElement = 0;
		descriptor_writes[1].descriptorType =
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[1].descriptorCount = 1;
		descriptor_writes[1].pImageInfo = &image_info;

		vkUpdateDescriptorSets(eng_data.logical_device,
							   static_cast<uint32_t>(descriptor_writes.size()),
							   descriptor_writes.data(),
							   0,
							   nullptr);
	}
}

void liboceanlight::engine::create_buffer(engine_data& eng_data,
										  VkDeviceSize size,
										  VkBufferUsageFlags usage,
										  VkMemoryPropertyFlags props,
										  VkBuffer& buff,
										  VkDeviceMemory& buff_mem)
{
	VkBufferCreateInfo buff_info {};
	buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buff_info.size = size;
	buff_info.usage = usage;
	buff_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult rv = vkCreateBuffer(eng_data.logical_device,
								 &buff_info,
								 nullptr,
								 &buff);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create vertex buffer");
	}

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(eng_data.logical_device, buff, &mem_reqs);

	uint32_t type_index = find_mem_type(eng_data,
										mem_reqs.memoryTypeBits,
										props);

	VkMemoryAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = type_index;

	rv = vkAllocateMemory(eng_data.logical_device,
						  &alloc_info,
						  nullptr,
						  &buff_mem);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate vertex buffer memory");
	}

	vkBindBufferMemory(eng_data.logical_device, buff, buff_mem, 0);
}

void liboceanlight::engine::copy_buffer(engine_data& eng_data,
										VkBuffer src,
										VkBuffer dst,
										VkDeviceSize size)
{
	VkCommandBuffer cmd_buffer {begin_single_time_cmds(eng_data)};
	VkBufferCopy copy_region {};
	copy_region.size = size;
	vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);
	end_single_time_cmds(eng_data, cmd_buffer);
}

uint32_t liboceanlight::engine::find_mem_type(engine_data& eng_data,
											  uint32_t type_filter,
											  VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(eng_data.physical_device, &mem_props);

	for (uint32_t i {0}; i < mem_props.memoryTypeCount; ++i)
	{
		if ((type_filter & (1 << i)) &&
			(gsl::at(mem_props.memoryTypes, i).propertyFlags & flags) == flags)
		{
			return i;
		}
	}

	throw std::runtime_error("Couldn't find suitable memory type");
}

void liboceanlight::engine::create_cmd_buffer(engine_data& eng_data)
{
	VkCommandBufferAllocateInfo alloc_info {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = eng_data.command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)eng_data.max_frames_in_flight;

	VkResult rv = vkAllocateCommandBuffers(eng_data.logical_device,
										   &alloc_info,
										   eng_data.command_buffers.data());
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer");
	}
}

void liboceanlight::engine::create_sync_objects(engine_data& eng_data)
{
	VkSemaphoreCreateInfo sem_info {};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult rv {};
	for (int i {0}; i < eng_data.max_frames_in_flight; ++i)
	{
		rv = vkCreateSemaphore(eng_data.logical_device,
							   &sem_info,
							   nullptr,
							   &gsl::at(eng_data.signal_sems, i));

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create signal semaphore");
		}

		rv = vkCreateSemaphore(eng_data.logical_device,
							   &sem_info,
							   nullptr,
							   &gsl::at(eng_data.wait_sems, i));

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create wait semaphore");
		}

		rv = vkCreateFence(eng_data.logical_device,
						   &fence_info,
						   nullptr,
						   &gsl::at(eng_data.in_flight_fences, i));

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create fence");
		}
	}
}

VkShaderModule liboceanlight::engine::create_shader(
	engine_data& eng_data,
	const std::vector<char>& shader_code)
{
	VkShaderModuleCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_code.size();
	create_info.pCode = static_cast<const uint32_t*>(
		static_cast<const void*>(shader_code.data()));

	VkShaderModule shader_module {};
	auto rv = vkCreateShaderModule(eng_data.logical_device,
								   &create_info,
								   nullptr,
								   &shader_module);
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module");
	}

	return shader_module;
}