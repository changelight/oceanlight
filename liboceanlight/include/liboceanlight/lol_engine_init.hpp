#ifndef LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_swapchain.hpp>

namespace liboceanlight::engine_init
{
	using engine_init_data = struct lol_engine_init_data
	{
		VkCommandPool command_pool {nullptr};
		VkImage depth_img {nullptr};
		VkDeviceMemory depth_img_mem {nullptr};
		VkImageView depth_img_view {nullptr};
		VkFormat depth_fmt {VK_FORMAT_D32_SFLOAT};
		std::vector<VkFramebuffer> frame_buffers;
	};
	void create_cmd_pool(void);
	uint32_t find_mem_type(uint32_t, VkMemoryPropertyFlags);
	void create_depth_image();
	VkCommandBuffer begin_single_time_cmds(void);
	void end_single_time_cmds(VkCommandBuffer);
	void transition_img_layout(VkImage,
							   VkFormat,
							   VkImageLayout,
							   VkImageLayout);
	void create_framebuffers(VkDevice,
							 VkRenderPass,
							 swapchain::swapchain_data&);
} /* namespace liboceanlight::engine_init */
extern liboceanlight::engine_init::engine_init_data init_data;

namespace liboceanlight::engine
{

	int init(window&, engine_data&);

	// void create_instance(engine_data&);
	// void create_physical_device(engine_data&);

	/* INSTANCE */
	// void set_inst_app_info(VkApplicationInfo&);
	// void set_inst_create_info(VkInstanceCreateInfo&);
	// std::vector<const char*> get_required_extensions();
	// void check_inst_ext_support(const std::vector<const char*>&);
	// void get_supported_inst_exts(std::vector<const char*>&);
	// void check_layer_support(const std::vector<const char*>&);
	// const std::vector<std::string> get_supported_layers();

	/* PHYSICAL_DEVICE */
	/*VkPhysicalDevice select_physical_dev(engine_data&,
										 std::vector<VkPhysicalDevice>&);*/
	// void check_dev_ext_support(engine_data&);
	// const std::vector<std::string> get_dev_exts(engine_data&);

	/* SURFACE */
	// void create_surface(window&, engine_data&);

	/* QUEUE */
	// void get_queue_fams(engine_data&);

	/* LOGICAL DEVICE */
	// void create_logical_device(engine_data&);

	/* SWAPCHAIN */
	// void get_swapchain_details(liboceanlight::window&, engine_data&);
	// VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR&);
	// void create_swapchain(engine_data&);
	// void create_image_views(engine_data&, liboceanlight::window&);
	/*VkImageView create_image_view(engine_data&,
								  VkImage,
								  VkFormat,
								  VkImageAspectFlags);*/

	/* PIPELINE */
	// void create_render_pass(engine_data&, liboceanlight::window&);
	// void create_descriptor_set_layout(engine_data&);
	// void create_pipeline(engine_data&);
	// VkShaderModule create_shader(engine_data&, const std::vector<char>&);
	// void create_framebuffers(engine_data&);
	void create_sync_objects(engine_data&);

	/* COMMAND */
	// void create_cmd_pool(engine_data&);
	void create_cmd_buffer(engine_data&);
	// VkCommandBuffer begin_single_time_cmds(engine_data&);
	//  void end_single_time_cmds(engine_data&, VkCommandBuffer&);

	/* TEXTURE */
	// void create_texture_img(engine_data&);
	/*void transition_img_layout(engine_data&,
							   VkImage,
							   VkFormat,
							   VkImageLayout,
							   VkImageLayout);*/
	// void copy_buffer_to_img(VkBuffer, VkImage, uint32_t, uint32_t);
	// void create_texture_sampler(VkDevice, float, VkSampler&);

	/* VERTEX BUFFER */
	// void create_vertex_buffers(engine_data&);
	/* void create_buffer(engine_data&,
					   VkDeviceSize,
					   VkBufferUsageFlags,
					   VkMemoryPropertyFlags,
					   VkBuffer&,
					   VkDeviceMemory&); */
	// void copy_buffer(engine_data&, VkBuffer, VkBuffer, VkDeviceSize);

	/* INDEX BUFFER */
	// void create_index_buffers(engine_data&);

	/* UNIFORM BUFFER */
	void create_uniform_buffers(engine_data&);

	/* DESCRIPTOR */
	void create_descriptor_pool(engine_data&);
	void create_descriptor_sets(engine_data&);

	/* DEPTH BUFFER */
	// void create_depth_resources(engine_data&);

	/* MODELS */
	// void load_models(engine_data&);
} /* namespace liboceanlight::engine */
#endif /* LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED */
