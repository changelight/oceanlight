#ifndef LOL_UTILITY_HPP_INCLUDED
#define LOL_UTILITY_HPP_INCLUDED
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace liboceanlight
{
	std::string queue_flags_to_string(const VkQueueFlags&);
	std::vector<char> read_file(const std::string&);
	int test_func(int, int);
} /* namespace liboceanlight */
#endif /* LOL_UTILITY_HPP_INCLUDED */
