#ifndef LIBOCEANLIGHT_UTIL_HPP_INCLUDED
#define LIBOCEANLIGHT_UTIL_HPP_INCLUDED
#include <iostream>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace liboceanlight
{
    /* Avoid using this in hotpaths for now since I don't know if the
    parameter unpacking is done via single-stepforwarding or recursion.
    If it is recursion, it will be slower. Also because we flush the stream. */
    template <typename... Args>
    std::ostream& generic_print(std::ostream& out, Args&&... args)
    {
        ((out << args << "\n"), ...) << std::endl;
        return out;
    }

    std::string version_string(void);
}

bool check_validation_layer_support(const std::vector<const char*>&);
VkApplicationInfo populate_vulkan_application_info();
VkInstanceCreateInfo populate_vulkan_create_info(VkApplicationInfo*);
VkDebugUtilsMessengerCreateInfoEXT populate_debug_utils_messenger_create_info();

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance,
    const VkDebugUtilsMessengerCreateInfoEXT*,
    const VkAllocationCallbacks*,
    VkDebugUtilsMessengerEXT*);

void DestroyDebugUtilsMessengerEXT(
    VkInstance,
    VkDebugUtilsMessengerEXT,
    const VkAllocationCallbacks*);

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT*,
    void*);
#endif /* LIBOCEANLIGHT_UTIL_HPP_INCLUDED */
