#ifndef LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>

namespace liboceanlight
{
	namespace engine
	{
		int init(window&, engine_data&);
		void create_instance(engine_data&);
		void create_physical_device(engine_data&);

		/* INSTANCE */
		void set_inst_app_info(VkApplicationInfo&);
		void set_inst_create_info(VkInstanceCreateInfo&);
		std::vector<const char*> get_required_extensions();
		void check_inst_ext_support(const std::vector<const char*>&);
		const std::vector<std::string> get_supported_inst_exts();
		void check_layer_support(const std::vector<const char*>&);
		const std::vector<std::string> get_supported_layers();

		/* PHYSICAL_DEVICE */
		VkPhysicalDevice select_physical_dev(std::vector<VkPhysicalDevice>&);
		void check_dev_ext_support(engine_data&);
		const std::vector<std::string> get_dev_exts(engine_data&);

		/* SURFACE */
		void create_surface(window&, engine_data&);

		/* QUEUE */
		void get_queue_fams(engine_data&);

		/* LOGICAL DEVICE */
		void create_logical_device(engine_data&);

		/* SWAPCHAIN */
		void get_swapchain_details(liboceanlight::window&, engine_data&);
		void create_swapchain(engine_data&);
		void create_image_views(engine_data&);

		/* PIPELINE */
		void create_render_pass(engine_data&);
		void create_pipeline(engine_data&);
		VkShaderModule create_shader(engine_data&, const std::vector<char>&);
		void create_framebuffers(engine_data&);
		void create_sync_objects(engine_data&);

		/* COMMAND */
		void create_cmd_pool(engine_data&);
		void create_cmd_buffer(engine_data&);

	} /* namespace engine */
} /* namespace liboceanlight */
#endif /* LIBOCEANLIGHT_ENGINE_INIT_HPP_INCLUDED */
