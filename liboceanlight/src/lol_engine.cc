#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <cstdint>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <map>
#include <optional>
#include <set>
#include <cstring>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_utility.hpp>
#include <config.h>

void liboceanlight::engine::run(liboceanlight::window& window)
{
	while (!window.should_close())
	{
		glfwWaitEvents();
		draw_frame();
	}

	vkDeviceWaitIdle(logical_device);
}

void liboceanlight::engine::draw_frame()
{
	vkWaitForFences(logical_device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(logical_device, 1, &in_flight_fence);

	uint32_t image_index;
	vkAcquireNextImageKHR(logical_device,
						  swap_chain,
						  UINT64_MAX,
						  image_available_semaphore,
						  VK_NULL_HANDLE,
						  &image_index);

	vkResetCommandBuffer(command_buffer, 0);
	record_command_buffer(command_buffer, image_index);

	VkSubmitInfo submit_info {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = {image_available_semaphore};
	VkPipelineStageFlags wait_stages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	VkSemaphore signal_semaphores[] = {rendering_finished_semaphore};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	auto rv = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer");
	}

	VkPresentInfoKHR present_info {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swap_chains[] = {swap_chain};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr;

	vkQueuePresentKHR(graphics_queue, &present_info);
}

void liboceanlight::engine::init(liboceanlight::window& window)
{
	vulkan_instance = create_vulkan_instance();
	window_surface = window.create_window_surface(vulkan_instance);
	physical_device = pick_physical_device();
	queue_family_indices = find_queue_families();

	if (!device_is_suitable(queue_family_indices,
							physical_device,
							window_surface,
							device_extensions))
	{
		throw std::runtime_error("Device lacks required queue family.");
	}

	logical_device = create_logical_device(physical_device,
										   queue_family_indices,
										   device_extensions);

	vkGetDeviceQueue(logical_device,
					 queue_family_indices.graphics_queue_family.value(),
					 0,
					 &graphics_queue);

	if (!graphics_queue)
	{
		throw std::runtime_error("Could not get device queue handle.");
	}

	vkGetDeviceQueue(logical_device,
					 queue_family_indices.presentation_queue_family.value(),
					 0,
					 &present_queue);

	if (!present_queue)
	{
		throw std::runtime_error("Could not get presentation queue handle.");
	}

	swap_chain = create_swap_chain(window, swap_details);
	swap_chain_image_views = create_image_views();
	create_render_pass();
	create_graphics_pipeline();
	create_framebuffers();
	create_command_pool();
	create_command_buffer();
	create_sync_objects();
}

/* Create the Vulkan instance */
VkInstance liboceanlight::engine::create_vulkan_instance()
{
	/* Optional - VkApplicationInfo is used for game-specific optimizations by
	 * hardware vendors (IHVs) */
	VkApplicationInfo instance_app_info;
	instance_app_info = populate_instance_app_info();

	/* Create and populate VkInstanceCreateInfo, used by VkCreateInstance to
	 * set basic parameters of the vulkan instance. We will fill in more as we
	 * go until passing to VkCreateInstance */
	VkInstanceCreateInfo instance_create_info;
	instance_create_info = populate_instance_create_info(instance_app_info);

	/* Get the instance extensions required by the windowing API */
	std::vector<const char*> extensions = get_required_instance_extensions();
	std::vector<const char*> layers {};

	/* If validation layers are enabled, check if they are supported. If they
	 * are, then set them up, along with the debug utils messenger */
	bool layers_supported {false}, extensions_supported {false};
	VkDebugUtilsMessengerCreateInfoEXT dbg_utils_msngr_create_info {};
	if (validation_layers_enabled)
	{
		layers.push_back("VK_LAYER_KHRONOS_validation");
		layers_supported = check_layer_support(layers);
		if (layers_supported)
		{
			setup_dbg_utils_msngr(extensions,
								  dbg_utils_msngr_create_info,
								  instance_create_info);
		}
		else
		{
			layers.pop_back();
		}
	}

	/* Set up our instance layers */
	setup_instance_layers(instance_create_info, layers);

	/* Check if all of our instance extensions are supported. If they are not,
	 * we have a problem */
	extensions_supported = check_extension_support(extensions);
	if (extensions_supported)
	{
		setup_instance_extensions(instance_create_info, extensions);
	}
	else
	{
		std::runtime_error("Missing some required instance extensions.");
	}

	/* Create our instance */
	VkInstance instance {nullptr};
	VkResult rv = vkCreateInstance(&instance_create_info, nullptr, &instance);

	if (rv != VK_SUCCESS || !instance)
	{
		throw std::runtime_error("Failed to create Vulkan instance.");
	}

	/* In vulkan, the debug utils messenger can be created twice. Once before
	 * instance creation, and once after. This is so we can print debug
	 * messages relevant to the instance creation itself. We've already used
	 * the "pre-instance" debug messenger by setting pNext in our struct, so
	 * now we will create the "post-instance" debug messenger, which will take
	 * over from here on out */
	if (layers_supported)
	{
		rv = CreateDebugUtilsMessengerEXT(instance,
										  &dbg_utils_msngr_create_info,
										  nullptr,
										  &debug_utils_messenger);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger.");
		}
	}

	return instance;
}

VkPhysicalDevice liboceanlight::engine::pick_physical_device()
{
	uint32_t device_count {0};
	VkPhysicalDevice physical_device {VK_NULL_HANDLE};
	vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);

	if (device_count == 0)
	{
		throw std::runtime_error("Failed to find supported device");
	}

	std::vector<VkPhysicalDevice> physical_devices(device_count);
	vkEnumeratePhysicalDevices(vulkan_instance,
							   &device_count,
							   physical_devices.data());

	std::multimap<uint32_t, VkPhysicalDevice> device_candidates;
	for (const auto& device : physical_devices)
	{
		uint32_t score = rate_device_suitability(device);
		device_candidates.insert(std::make_pair(score, device));

		/* Multimap is sorted, so rbegin()->first contains highest score */
		if (device_candidates.rbegin()->first > 0)
		{
			physical_device = device_candidates.rbegin()->second;
		}
		else
		{
			throw std::runtime_error("Failed to find suitable device.");
		}
	}

	if (physical_device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Found devices, but none suitable.");
	}

	return physical_device;
}

VkDevice create_logical_device(
	VkPhysicalDevice& physical_device,
	queue_family_indices_struct& indices,
	const std::vector<const char*>& device_extensions)
{
	VkDevice logical_device {nullptr};

	std::set<uint32_t> unique_queue_families {
		indices.graphics_queue_family.value(),
		indices.presentation_queue_family.value()};
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	for (uint32_t queue_family : unique_queue_families)
	{
		queue_create_infos.push_back(populate_queue_create_info(queue_family));
	}

	VkPhysicalDeviceFeatures features {};
	VkDeviceCreateInfo device_create_info;
	device_create_info = populate_device_create_info(features,
													 queue_create_infos,
													 device_extensions);

	VkResult rv;
	rv = vkCreateDevice(physical_device,
						&device_create_info,
						nullptr,
						&logical_device);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Logical device creation failed");
	}

	return logical_device;
}

struct swap_chain_support_details get_swap_chain_support_details(
	VkPhysicalDevice& device,
	VkSurfaceKHR& surface)
{
	swap_chain_support_details details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,
											  surface,
											  &details.capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device,
										 surface,
										 &format_count,
										 nullptr);

	if (format_count == 0)
	{
		throw std::runtime_error(
			"Could not get physical device surface formats");
	}

	details.formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device,
										 surface,
										 &format_count,
										 details.formats.data());

	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device,
											  surface,
											  &present_mode_count,
											  nullptr);

	if (present_mode_count == 0)
	{
		throw std::runtime_error(
			"Could not get physical device present modes");
	}

	details.present_modes.resize(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device,
											  surface,
											  &present_mode_count,
											  details.present_modes.data());

	return details;
}

VkSurfaceFormatKHR choose_swap_surface_format(
	const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	for (const auto& available_format : available_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return available_format;
		}
	}

	return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(
	const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return available_present_mode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D liboceanlight::window::choose_swap_extent(
	const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width !=
		std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window_pointer, &width, &height);

		VkExtent2D actual_extent = {static_cast<uint32_t>(width),
									static_cast<uint32_t>(height)};

		actual_extent.width = std::clamp(actual_extent.width,
										 capabilities.minImageExtent.width,
										 capabilities.maxImageExtent.width);

		actual_extent.height = std::clamp(actual_extent.height,
										  capabilities.minImageExtent.height,
										  capabilities.maxImageExtent.height);

		return actual_extent;
	}
}

VkSwapchainKHR liboceanlight::engine::create_swap_chain(
	liboceanlight::window& window,
	swap_chain_support_details& details)
{
	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(
		swap_details.formats);
	VkPresentModeKHR present_mode = choose_swap_present_mode(
		swap_details.present_modes);
	swap_chain_extent = window.choose_swap_extent(swap_details.capabilities);
	swap_chain_image_format = surface_format.format;

	uint32_t min_image_count = swap_details.capabilities.minImageCount;
	uint32_t image_count = min_image_count + 1;
	uint32_t max_image_count = swap_details.capabilities.maxImageCount;

	if (max_image_count > 0 && image_count > max_image_count)
	{
		image_count = max_image_count;
	}

	VkSwapchainCreateInfoKHR create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = window_surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = swap_chain_image_format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = swap_chain_extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t indices[] = {
		queue_family_indices.graphics_queue_family.value(),
		queue_family_indices.presentation_queue_family.value()};

	if (queue_family_indices.graphics_queue_family !=
		queue_family_indices.presentation_queue_family)
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = indices;
	}
	else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	}

	create_info.preTransform = swap_details.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR sw {nullptr};

	if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &sw) !=
		VK_SUCCESS)
	{
		std::runtime_error("Failed to create swap chain");
	}

	vkGetSwapchainImagesKHR(logical_device, sw, &image_count, nullptr);
	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(logical_device,
							sw,
							&image_count,
							swap_chain_images.data());

	return sw;
}

std::vector<VkImageView> liboceanlight::engine::create_image_views()
{
	std::vector<VkImageView> image_views;
	image_views.resize(swap_chain_images.size());

	for (size_t i {0}; i < swap_chain_images.size(); ++i)
	{
		VkImageViewCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = swap_chain_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = swap_chain_image_format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(logical_device,
							  &create_info,
							  nullptr,
							  &image_views[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views.");
		}
	}

	return image_views;
}

void liboceanlight::engine::create_graphics_pipeline()
{
	auto vertex_shader_code = read_file(
		"../liboceanlight/shaders/vertex_shader.spv");
	auto fragment_shader_code = read_file(
		"../liboceanlight/shaders/fragment_shader.spv");

	VkShaderModule vertex_shader = create_shader_module(vertex_shader_code);
	VkShaderModule fragment_shader = create_shader_module(
		fragment_shader_code);

	VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info {};
	vertex_shader_stage_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_stage_create_info.module = vertex_shader;
	vertex_shader_stage_create_info.pName = "main";

	VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info {};
	fragment_shader_stage_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_stage_create_info.module = fragment_shader;
	fragment_shader_stage_create_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		vertex_shader_stage_create_info,
		fragment_shader_stage_create_info};

	VkPipelineVertexInputStateCreateInfo vertex_input_create_info {};
	vertex_input_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_info.vertexBindingDescriptionCount = 0;
	vertex_input_create_info.pVertexBindingDescriptions = nullptr;
	vertex_input_create_info.vertexAttributeDescriptionCount = 0;
	vertex_input_create_info.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info {};
	input_assembly_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swap_chain_extent.width;
	viewport.height = (float)swap_chain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor {};
	scissor.offset = {0, 0};
	scissor.extent = swap_chain_extent;

	std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
												  VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
	dynamic_state_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(
		dynamic_states.size());
	dynamic_state_create_info.pDynamicStates = dynamic_states.data();

	VkPipelineViewportStateCreateInfo viewport_state_create_info {};
	viewport_state_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info {};
	rasterizer_state_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_state_create_info.depthClampEnable = VK_FALSE;
	rasterizer_state_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_state_create_info.lineWidth = 1.0f;
	rasterizer_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer_state_create_info.depthBiasEnable = VK_FALSE;
	rasterizer_state_create_info.depthBiasConstantFactor = 0.0f;
	rasterizer_state_create_info.depthBiasClamp = 0.0f;
	rasterizer_state_create_info.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info {};
	multisampling_state_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling_state_create_info.sampleShadingEnable = VK_FALSE;
	multisampling_state_create_info.rasterizationSamples =
		VK_SAMPLE_COUNT_1_BIT;
	multisampling_state_create_info.minSampleShading = 1.0f;
	multisampling_state_create_info.pSampleMask = nullptr;
	multisampling_state_create_info.alphaToCoverageEnable = VK_FALSE;
	multisampling_state_create_info.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState
		color_blend_attachment_state_create_info {};
	color_blend_attachment_state_create_info.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment_state_create_info.blendEnable = VK_FALSE;
	color_blend_attachment_state_create_info.srcColorBlendFactor =
		VK_BLEND_FACTOR_ONE;
	color_blend_attachment_state_create_info.dstColorBlendFactor =
		VK_BLEND_FACTOR_ZERO;
	color_blend_attachment_state_create_info.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state_create_info.srcAlphaBlendFactor =
		VK_BLEND_FACTOR_ONE;
	color_blend_attachment_state_create_info.dstAlphaBlendFactor =
		VK_BLEND_FACTOR_ZERO;
	color_blend_attachment_state_create_info.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
	color_blend_state_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.logicOpEnable = VK_FALSE;
	color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_state_create_info.attachmentCount = 1;
	color_blend_state_create_info.pAttachments =
		&color_blend_attachment_state_create_info;
	color_blend_state_create_info.blendConstants[0] = 0.0f;
	color_blend_state_create_info.blendConstants[1] = 0.0f;
	color_blend_state_create_info.blendConstants[2] = 0.0f;
	color_blend_state_create_info.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
	pipeline_layout_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 0;
	pipeline_layout_create_info.pSetLayouts = nullptr;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;

	auto rv = vkCreatePipelineLayout(logical_device,
									 &pipeline_layout_create_info,
									 nullptr,
									 &pipeline_layout);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout");
	}

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info {};
	graphics_pipeline_create_info.sType =
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphics_pipeline_create_info.stageCount = 2;
	graphics_pipeline_create_info.pStages = shader_stages;
	graphics_pipeline_create_info.pVertexInputState =
		&vertex_input_create_info;
	graphics_pipeline_create_info.pInputAssemblyState =
		&input_assembly_create_info;
	graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
	graphics_pipeline_create_info.pRasterizationState =
		&rasterizer_state_create_info;
	graphics_pipeline_create_info.pMultisampleState =
		&multisampling_state_create_info;
	graphics_pipeline_create_info.pDepthStencilState = nullptr;
	graphics_pipeline_create_info.pColorBlendState =
		&color_blend_state_create_info;
	graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
	graphics_pipeline_create_info.layout = pipeline_layout;
	graphics_pipeline_create_info.renderPass = render_pass;
	graphics_pipeline_create_info.subpass = 0;
	graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	graphics_pipeline_create_info.basePipelineIndex = -1;

	rv = vkCreateGraphicsPipelines(logical_device,
								   VK_NULL_HANDLE,
								   1,
								   &graphics_pipeline_create_info,
								   NULL,
								   &graphics_pipeline);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	vkDestroyShaderModule(logical_device, vertex_shader, nullptr);
	vkDestroyShaderModule(logical_device, fragment_shader, nullptr);
}

void liboceanlight::engine::create_render_pass()
{
	VkAttachmentDescription color_attachment_description {};
	color_attachment_description.format = swap_chain_image_format;
	color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_description.stencilLoadOp =
		VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_description.stencilStoreOp =
		VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_reference {};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_reference;

	VkSubpassDependency subpass_dependency {};
	subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.srcAccessMask = 0;
	subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_create_info {};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment_description;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass_description;
	render_pass_create_info.dependencyCount = 1;
	render_pass_create_info.pDependencies = &subpass_dependency;

	auto rv = vkCreateRenderPass(logical_device,
								 &render_pass_create_info,
								 nullptr,
								 &render_pass);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}
}

void liboceanlight::engine::create_framebuffers()
{
	swap_chain_frame_buffers.resize(swap_chain_image_views.size());

	for (size_t i {0}; i < swap_chain_image_views.size(); ++i)
	{
		VkImageView attachments[] = {swap_chain_image_views[i]};

		VkFramebufferCreateInfo frame_buffer_create_info {};
		frame_buffer_create_info.sType =
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = render_pass;
		frame_buffer_create_info.attachmentCount = 1;
		frame_buffer_create_info.pAttachments = attachments;
		frame_buffer_create_info.width = swap_chain_extent.width;
		frame_buffer_create_info.height = swap_chain_extent.height;
		frame_buffer_create_info.layers = 1;

		auto rv = vkCreateFramebuffer(logical_device,
									  &frame_buffer_create_info,
									  nullptr,
									  &swap_chain_frame_buffers[i]);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image frambuffers");
		}
	}
}

void liboceanlight::engine::create_command_pool()
{
	VkCommandPoolCreateInfo command_pool_create_info {};
	command_pool_create_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.flags =
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex =
		queue_family_indices.graphics_queue_family.value();

	auto rv = vkCreateCommandPool(logical_device,
								  &command_pool_create_info,
								  nullptr,
								  &command_pool);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool");
	}
}

void liboceanlight::engine::create_command_buffer()
{
	VkCommandBufferAllocateInfo command_buffer_allocate_create_info {};
	command_buffer_allocate_create_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_create_info.commandPool = command_pool;
	command_buffer_allocate_create_info.level =
		VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_create_info.commandBufferCount = 1;

	auto rv = vkAllocateCommandBuffers(logical_device,
									   &command_buffer_allocate_create_info,
									   &command_buffer);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}
}

void liboceanlight::engine::record_command_buffer(
	VkCommandBuffer& command_buffer,
	uint32_t image_index)
{
	VkCommandBufferBeginInfo command_buffer_begin_info {};
	command_buffer_begin_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = 0;
	command_buffer_begin_info.pInheritanceInfo = nullptr;

	auto rv = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer");
	}

	VkRenderPassBeginInfo render_pass_begin_info {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = render_pass;
	render_pass_begin_info.framebuffer = swap_chain_frame_buffers[image_index];
	render_pass_begin_info.renderArea.offset = {0, 0};
	render_pass_begin_info.renderArea.extent = swap_chain_extent;

	VkClearValue clear_value = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = &clear_value;

	vkCmdBeginRenderPass(command_buffer,
						 &render_pass_begin_info,
						 VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffer,
					  VK_PIPELINE_BIND_POINT_GRAPHICS,
					  graphics_pipeline);

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swap_chain_extent.width);
	viewport.height = static_cast<float>(swap_chain_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor {};
	scissor.offset = {0, 0};
	scissor.extent = swap_chain_extent;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	vkCmdDraw(command_buffer, 3, 1, 0, 0);
	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to end command buffer");
	}
}

void liboceanlight::engine::create_sync_objects()
{
	VkSemaphoreCreateInfo semaphore_create_info {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_create_info {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	auto rv1 = vkCreateSemaphore(logical_device,
								 &semaphore_create_info,
								 nullptr,
								 &image_available_semaphore);

	auto rv2 = vkCreateSemaphore(logical_device,
								 &semaphore_create_info,
								 nullptr,
								 &rendering_finished_semaphore);

	auto rv3 = vkCreateFence(logical_device,
							 &fence_create_info,
							 nullptr,
							 &in_flight_fence);

	if (rv1 != VK_SUCCESS || rv2 != VK_SUCCESS || rv3 != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create synchronization objects");
	}
}

VkShaderModule liboceanlight::engine::create_shader_module(
	const std::vector<char>& shader_code)
{
	VkShaderModuleCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());

	VkShaderModule shader_module;
	auto rv = vkCreateShaderModule(logical_device,
								   &create_info,
								   nullptr,
								   &shader_module);
	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module");
	}

	return shader_module;
}

uint32_t rate_device_suitability(const VkPhysicalDevice& physical_device)
{
	uint32_t score {0};

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(physical_device, &device_features);

	if (!device_features.geometryShader)
	{
		return 0;
	}

	if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 100;
	}

	score += device_properties.limits.maxImageDimension2D;

	return score;
}

queue_family_indices_struct liboceanlight::engine::find_queue_families()
{
	queue_family_indices_struct indices;
	VkBool32 present_support {false};
	uint32_t queue_family_count {0};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
											 &queue_family_count,
											 nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
											 &queue_family_count,
											 queue_families.data());

	std::cout << "Queue families are as follows:\n";

	VkResult rv;
	int i {0};
	for (const auto& queue_family : queue_families)
	{
		std::cout << "Queue Count: " << queue_family.queueCount << "\n"
				  << "Queue Type: "
				  << queue_flags_to_string(queue_family.queueFlags) << "\n";

		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphics_queue_family = i;
		}

		rv = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
												  i,
												  window_surface,
												  &present_support);

		if (rv != VK_SUCCESS)
		{
			throw std::runtime_error("Could not determine surface support");
		}

		if (present_support)
		{
			indices.presentation_queue_family = i;
		}

		++i;
	}

	return indices;
}

bool check_device_extension_support(
	VkPhysicalDevice& device,
	const std::vector<const char*>& device_extensions)
{
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device,
										 nullptr,
										 &extension_count,
										 nullptr);

	std::vector<VkExtensionProperties> extension_properties(extension_count);
	vkEnumerateDeviceExtensionProperties(device,
										 nullptr,
										 &extension_count,
										 extension_properties.data());

	std::set<std::string> required_extensions(device_extensions.begin(),
											  device_extensions.end());

	for (const auto& extension : extension_properties)
	{
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}

bool liboceanlight::engine::device_is_suitable(
	queue_family_indices_struct& indices,
	VkPhysicalDevice& device,
	VkSurfaceKHR& window_surface,
	const std::vector<const char*>& device_extensions)
{
	bool extensions_supported = check_device_extension_support(
		device,
		device_extensions);

	bool swap_chain_adequate = false;
	if (extensions_supported)
	{
		swap_details = get_swap_chain_support_details(device, window_surface);

		swap_chain_adequate = !swap_details.formats.empty() &&
							  !swap_details.present_modes.empty();
	}

	return indices.graphics_queue_family.has_value() &&
		   indices.presentation_queue_family.has_value() &&
		   extensions_supported && swap_chain_adequate;
}

void setup_dbg_utils_msngr(
	std::vector<const char*>& extensions,
	VkDebugUtilsMessengerCreateInfoEXT& dbg_utils_msngr_create_info,
	VkInstanceCreateInfo& instance_create_info)
{
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	dbg_utils_msngr_create_info = populate_dbg_utils_msngr_create_info();
	instance_create_info.pNext =
		(VkDebugUtilsMessengerCreateInfoEXT*)&dbg_utils_msngr_create_info;
}

void setup_instance_layers(VkInstanceCreateInfo& c_info,
						   std::vector<const char*>& layers)
{
	if (layers.size() > 0)
	{
		c_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
		c_info.ppEnabledLayerNames = layers.data();
	}
}

void setup_instance_extensions(VkInstanceCreateInfo& c_info,
							   std::vector<const char*>& extensions)
{
	c_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	c_info.ppEnabledExtensionNames = extensions.data();
}

bool check_layer_support(const std::vector<const char*>& layers)
{
	uint32_t layer_count {0};
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> layer_properties(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());

	int layers_found {0};
	bool layer_found, all_layers_found {false};
	for (const char* layer : layers)
	{
		layer_found = false;

		for (const auto& i : layer_properties)
		{
			if (strcmp(layer, i.layerName) == 0)
			{
				layer_found = true;
				++layers_found;
				std::cout << "Layer Found: " << layer << "\n";
				break;
			}
		}

		if (!layer_found)
		{
			std::cerr << "Layer Not Found: " << layer << "\n";
		}
	}

	if (layers_found == layers.size())
	{
		all_layers_found = true;
		std::cout << "ALL INSTANCE LAYERS FOUND\n";
	}

	return all_layers_found;
}

bool check_extension_support(const std::vector<const char*>& extensions)
{
	uint32_t extension_count {0};
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> extension_properties(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr,
										   &extension_count,
										   extension_properties.data());

	int extensions_found {0};
	bool extension_found, all_extensions_found {false};
	for (const char* extension : extensions)
	{
		extension_found = false;

		for (const auto& i : extension_properties)
		{
			if (strcmp(extension, i.extensionName) == 0)
			{
				extension_found = true;
				++extensions_found;
				std::cout << "Extension Found: " << extension << "\n";
			}
		}

		if (!extension_found)
		{
			std::cerr << "Extension Not Found: " << extension << "\n";
		}
	}

	if (extensions_found == extensions.size())
	{
		all_extensions_found = true;
		std::cout << "ALL INSTANCE EXTENSIONS FOUND\n";
	}

	return all_extensions_found;
}

std::vector<const char*> get_required_instance_extensions()
{
	uint32_t count {0};
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions_vec(extensions, extensions + count);
	return extensions_vec;
}

VkInstanceCreateInfo populate_instance_create_info(VkApplicationInfo& app_info)
{
	VkInstanceCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.pNext = nullptr;
	return create_info;
}

VkApplicationInfo populate_instance_app_info()
{
	VkApplicationInfo app_info {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = PROJECT_NAME;
	app_info.pEngineName = PROJECT_NAME;
	app_info.apiVersion = VK_API_VERSION_1_3;
	app_info.pNext = nullptr;

	app_info.applicationVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR,
												  PROJECT_VER_MINOR,
												  PROJECT_VER_PATCH);

	app_info.engineVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR,
											 PROJECT_VER_MINOR,
											 PROJECT_VER_PATCH);

	return app_info;
}

VkDeviceQueueCreateInfo populate_queue_create_info(uint32_t& queue_family)
{
	VkDeviceQueueCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	create_info.queueFamilyIndex = queue_family;
	create_info.queueCount = 1;
	float queue_priority = 1.0f;
	create_info.pQueuePriorities = &queue_priority;
	return create_info;
}

VkDeviceCreateInfo populate_device_create_info(
	VkPhysicalDeviceFeatures& features,
	std::vector<VkDeviceQueueCreateInfo>& queue_create_infos,
	const std::vector<const char*>& device_extensions)
{
	VkDeviceCreateInfo create_info {};
	create_info.queueCreateInfoCount = static_cast<uint32_t>(
		queue_create_infos.size());
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pEnabledFeatures = &features;
	create_info.enabledExtensionCount = static_cast<uint32_t>(
		device_extensions.size());
	create_info.ppEnabledExtensionNames = device_extensions.data();

	return create_info;
}
