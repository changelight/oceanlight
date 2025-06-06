#include <array>
#include <stdexcept>
#include <liboceanlight/lol_device.hpp>
#include <liboceanlight/lol_pipeline.hpp>
#include <liboceanlight/lol_window.hpp>

liboceanlight::pipeline::pipeline_data pipe_data;

void liboceanlight::pipeline::create_render_pass(liboceanlight::window& w)
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

	VkAttachmentDescription depth_attachment {};
	depth_attachment.format = pipe_data.depth_fmt;
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

	VkResult rv = vkCreateRenderPass(dev_data.logical_device,
									 &render_pass_info,
									 nullptr,
									 &pipe_data.render_pass);

	if (rv != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass");
	}
}
