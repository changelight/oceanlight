#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <liboceanlight/lol_utility.hpp>

std::string liboceanlight::queue_flags_to_string(const VkQueueFlags& flags)
{
	std::stringstream formatted;

	std::map<unsigned int, std::string> flagbits
	{
		{VK_QUEUE_GRAPHICS_BIT, "Graphics"},
        {VK_QUEUE_COMPUTE_BIT, "Compute"},
		{VK_QUEUE_TRANSFER_BIT, "Transfer"},
		{VK_QUEUE_SPARSE_BINDING_BIT, "Sparsebinding"},
		{VK_QUEUE_PROTECTED_BIT, "Protected"},
		{VK_QUEUE_VIDEO_DECODE_BIT_KHR, "Video Decode"},
    #ifdef VK_ENABLE_BETA_EXTENSIONS
		{VK_QUEUE_VIDEO_ENCODE , "Video Encode"},
    #endif
        {VK_QUEUE_OPTICAL_FLOW_BIT_NV, "Optical Flow"}
	};
    
    for (const auto& flagbit : flagbits)
    {
        if (flags & flagbit.first)
            formatted << "|" << flagbit.second;
    }

	return formatted.str() + "|";
}