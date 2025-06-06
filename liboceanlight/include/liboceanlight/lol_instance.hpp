#ifndef LIBOCEANLIGHT_INSTANCE_HPP_INCLUDED
#define LIBOCEANLIGHT_INSTANCE_HPP_INCLUDED
#include <vulkan/vulkan_core.h>

namespace liboceanlight::instance
{
	using instance_data = struct lol_instance_data_struct
	{
		/* INSTANCE */
		VkDebugUtilsMessengerEXT dbg_messenger {nullptr};
		VkInstance vulkan_instance {nullptr};
#ifdef NDEBUG
		bool validation_layer_enabled {false};
#else
		bool validation_layer_enabled {true};
#endif /* NDEBUG */
	};
	int create_instance(void);
} /* namespace liboceanlight::instance */
extern liboceanlight::instance::instance_data inst_data;
#endif /* LIBOCEANLIGHT_INSTANCE_HPP_INCLUDED */
