#include <iostream>
#include <vector>
#include <map>
#include <limits>
#include <optional>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/engine.hpp>
#include <liboceanlight/util.hpp>
#include <config.h>

void liboceanlight::engine::init()
{
    VkResult result;
    VkApplicationInfo app_info = populate_vulkan_application_info();
    VkInstanceCreateInfo create_info = populate_vulkan_create_info(&app_info);

    uint32_t required_extension_count {0};
    const char** initial_required_extensions = glfwGetRequiredInstanceExtensions(&required_extension_count);
    std::vector<const char*> required_extensions(initial_required_extensions, initial_required_extensions + required_extension_count);
    const std::vector<const char*> validation_layers {"VK_LAYER_KHRONOS_validation"};

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info {};
    if (validation_layers_enable)
    {
        bool validation_layer_support = check_validation_layer_support(validation_layers);
        if (!validation_layer_support)
        {
            throw std::runtime_error("Validation layers requested, but none available.");
        }

        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        debug_utils_messenger_create_info = populate_debug_utils_messenger_create_info();
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_utils_messenger_create_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    uint32_t vulkan_extension_count {0};
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, nullptr);

    std::vector<VkExtensionProperties> vulkan_extensions(vulkan_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, vulkan_extensions.data());

    result = vkCreateInstance(&create_info, nullptr, &vulkan_instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance.");
    }

    if (validation_layers_enable)
    {
        result = CreateDebugUtilsMessengerEXT(vulkan_instance, &debug_utils_messenger_create_info, nullptr, &debug_utils_messenger);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger.");
        }
    }
    
    VkPhysicalDevice physical_device = pick_physical_device();

    if (!check_device_queue_family_support(physical_device))
    {
        throw std::runtime_error("Found suitable device, but lacks the required queue family");
    }
}

VkPhysicalDevice liboceanlight::engine::pick_physical_device()
{
    uint32_t device_count {0};
    VkPhysicalDevice physical_device {VK_NULL_HANDLE};
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);

    if (device_count == 0)
    {
        throw std::runtime_error("Failed to find physical device with vulkan support.");
    }

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, physical_devices.data());
    
    std::multimap<uint32_t, VkPhysicalDevice> device_candidates;
    for (const auto& device : physical_devices)
    {
        uint32_t score = rate_device_suitability(device);
        device_candidates.insert(std::make_pair(score, device));

        if (device_candidates.rbegin()->first > 0)
        {
            physical_device = device_candidates.rbegin()->second;
        }
        else
        {
            throw std::runtime_error("Failed to find suitable graphics device.");
        }
    }

    if (physical_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Found physical devices, but none suitable for our operations.");
    }

    return physical_device;
}

uint32_t liboceanlight::engine::rate_device_suitability(VkPhysicalDevice physical_device)
{
    uint32_t score {0};

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    if (!device_features.geometryShader)
    {
        return 0;
    }

    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 100;
    }

    score += device_properties.limits.maxImageDimension2D;

    return score;
}

liboceanlight::queue_family_indices_struct liboceanlight::engine::find_queue_families(VkPhysicalDevice device)
{
    liboceanlight::queue_family_indices_struct indices;

    uint32_t queue_family_count {0};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i {0};
    for (const auto& queue_family : queue_families)
    {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
            break;
        }

        ++i;
    }

    return indices;
}

bool liboceanlight::engine::check_device_queue_family_support(VkPhysicalDevice device)
{
    liboceanlight::queue_family_indices_struct indices = find_queue_families(device);
    return indices.graphics_family.has_value();
}

void liboceanlight::engine::run(liboceanlight::window& window)
{
    while (!window.should_close())
    {
        //glfwSwapBuffers(window);
        glfwWaitEvents();
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void error_callback(int code, const char* description)
{
    std::cerr << "Error " << code << ": " << description << "\n";
}
