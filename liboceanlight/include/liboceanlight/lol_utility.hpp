#ifndef LOL_UTILITY_HPP_INCLUDED
#define LOL_UTILITY_HPP_INCLUDED
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_engine.hpp>

namespace liboceanlight::utility
{
	std::string queue_flags_to_string(const VkQueueFlags&);
	const std::vector<char> read_file(const std::string&);
	void copy_buffer(VkBuffer, VkBuffer, VkDeviceSize);
	void copy_buffer_to_img(VkBuffer, VkImage, uint32_t, uint32_t);
	uint32_t find_mem_type(VkPhysicalDevice, uint32_t, VkMemoryPropertyFlags);
	int test_func(int, int);
} /* namespace liboceanlight::utility */
#endif /* LOL_UTILITY_HPP_INCLUDED */
