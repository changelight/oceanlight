#ifndef LIBOCEANLIGHT_RESOURCE_HPP_INCLUDED
#define LIBOCEANLIGHT_RESOURCE_HPP_INCLUDED
#include <vector>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_window.hpp>

namespace liboceanlight::resource
{
	using resource_data = struct lol_resource_data_struct
	{
	};
	VkShaderModule create_shader(VkDevice, const std::vector<char>&);
} /* namespace liboceanlight::resource */
extern liboceanlight::resource::resource_data res_data;
#endif /* LIBOCEANLIGHT_RESOURCE_HPP_INCLUDED */
