#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <liboceanlight/engine.hpp>
#include <liboceanlight/util.hpp>
#include <config.h>

std::string liboceanlight::version_string()
{
    return PROJECT_NAME " Ver. " PROJECT_VER;
}

bool check_validation_layer_support(const std::vector<const char*>& validation_layers)
{
    uint32_t layer_count {0};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    // Maybe there's a better way to do this
    for (const char* layer_name : validation_layers)
    {
        bool layer_found = false;

        for (const auto& layer_properties : available_layers)
        {
            if (strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                std::cout << "Found Validation Layer: \n" << layer_name << "\n";
                break;
            }
        }

        if (!layer_found)
        {
            std::cout << "LAYER NOT FOUND: " << layer_name << "\n";
            return false;
        }
    }
    return true;
}

VkApplicationInfo populate_vulkan_application_info()
{
    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = PROJECT_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR, PROJECT_VER_MINOR, PROJECT_VER_PATCH);
    app_info.pEngineName = PROJECT_NAME;
    app_info.engineVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR, PROJECT_VER_MINOR, PROJECT_VER_PATCH);
    app_info.apiVersion = VK_API_VERSION_1_2;
    return app_info;
}

VkInstanceCreateInfo populate_vulkan_create_info(VkApplicationInfo* app_info)
{
    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.flags = 0;
    create_info.pApplicationInfo = app_info;
    return create_info;
}

VkDebugUtilsMessengerCreateInfoEXT populate_debug_utils_messenger_create_info()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_utils_messenger_callback;
    create_info.pUserData = nullptr; // Optional
    return create_info;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        throw std::runtime_error("Vulkan validation layer detected a fatal error");
    }
    else
    {
        std::cout << callback_data->pMessage << "\n";
    }
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}
