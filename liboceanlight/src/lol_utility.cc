#include <fstream>
#include <iostream>
#include <liboceanlight/lol_utility.hpp>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

std::string liboceanlight::queue_flags_to_string(const VkQueueFlags& flags)
{
	std::stringstream formatted;

	std::map<unsigned int, std::string> flagbits {
		{VK_QUEUE_GRAPHICS_BIT, "Graphics"},
		{VK_QUEUE_COMPUTE_BIT, "Compute"},
		{VK_QUEUE_TRANSFER_BIT, "Transfer"},
		{VK_QUEUE_SPARSE_BINDING_BIT, "Sparsebinding"},
		{VK_QUEUE_PROTECTED_BIT, "Protected"},
		{VK_QUEUE_VIDEO_DECODE_BIT_KHR, "Video Decode"},
#ifdef VK_ENABLE_BETA_EXTENSIONS
		{VK_QUEUE_VIDEO_ENCODE, "Video Encode"},
#endif
		{VK_QUEUE_OPTICAL_FLOW_BIT_NV, "Optical Flow"}};

	for (const auto& flagbit : flagbits)
	{
		if (flags & flagbit.first)
			formatted << "|" << flagbit.second;
	}

	return formatted.str() + "|";
}

std::vector<char> liboceanlight::read_file(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file " + filename);
	}

	const auto filesize {file.tellg()};
	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);
	file.close();

	return buffer;
}

int liboceanlight::test_func(int a, int b)
{
	return a + b;
}
