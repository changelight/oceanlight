#ifndef LOL_UTILITY_HPP_INCLUDED
#define LOL_UTILITY_HPP_INCLUDED
#include <vulkan/vulkan_core.h>
#include <string>
namespace liboceanlight
{
    std::string queue_flags_to_string(const VkQueueFlags&);
}
#endif /* LOL_UTILITY_HPP_INCLUDED */