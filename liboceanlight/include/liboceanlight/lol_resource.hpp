#ifndef LIBOCEANLIGHT_RESOURCE_HPP_INCLUDED
#define LIBOCEANLIGHT_RESOURCE_HPP_INCLUDED
#include <array>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_engine.hpp>

namespace liboceanlight::resource
{
	using resource_data = struct lol_resource_data_struct
	{
	};
	VkShaderModule shader(VkDevice, const std::vector<char>&);
	void image(uint32_t,
			   uint32_t,
			   VkFormat,
			   VkImageTiling,
			   VkImageUsageFlags,
			   VkMemoryPropertyFlags,
			   VkImage&,
			   VkDeviceMemory&);
	void buffer(VkDeviceSize,
				VkBufferUsageFlags,
				VkMemoryPropertyFlags,
				VkBuffer&,
				VkDeviceMemory&);
	void texture_from_file(const std::string_view&, texture::lol_texture&);
	void texture_sampler(float, VkSampler&);
	void model_from_obj(const char*, models::lol_model&);
	void uniform_buffer(VkBuffer&, VkDeviceMemory&, void**);
	void descriptor_pool(VkDescriptorPoolSize*,
						 uint32_t,
						 engine::engine_data&);
	// void descriptor_set(engine::engine_data&);
	VkDescriptorSetLayoutBinding layout_binding(uint32_t,
												VkDescriptorType,
												uint32_t,
												VkShaderStageFlags);
	void descriptor_sets(VkDevice&,
						 VkDescriptorPool&,
						 VkDescriptorSetLayout*,
						 unsigned int,
						 std::array<VkDescriptorSet, 2>&);
	void command_buffer(VkDevice&, VkCommandPool&, uint32_t, VkCommandBuffer*);
	void load_models(std::vector<liboceanlight::models::lol_model>&);

} /* namespace liboceanlight::resource */
extern liboceanlight::resource::resource_data res_data;
#endif /* LIBOCEANLIGHT_RESOURCE_HPP_INCLUDED */
