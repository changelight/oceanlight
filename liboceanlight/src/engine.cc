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
    VkApplicationInfo instance_app_info;
    instance_app_info = populate_instance_app_info();

    VkInstanceCreateInfo instance_create_info;
    instance_create_info = populate_instance_create_info(&instance_app_info);

    std::vector<const char*> extensions = get_required_instance_extensions();
    std::vector<const char*> vldn_layers {"VK_LAYER_KHRONOS_validation"};

    VkDebugUtilsMessengerCreateInfoEXT dbg_utils_msngr_create_info {};
    if (validation_layers_enabled && check_vldn_layer_support(vldn_layers))
    {
        enable_vldn_layers(instance_create_info, vldn_layers);
        enable_dbg_utils_msngr(
            extensions,
            dbg_utils_msngr_create_info,
            instance_create_info);
    }

    instance_create_info.enabledExtensionCount = extensions.size();
    instance_create_info.ppEnabledExtensionNames = extensions.data();

    uint32_t vulkan_extension_count {0};
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, nullptr);

    std::vector<VkExtensionProperties> vulkan_extensions(vulkan_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, vulkan_extensions.data());

    VkResult result = vkCreateInstance(
        &instance_create_info,
        nullptr,
        &vulkan_instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance.");
    }

    if (validation_layers_enabled)
    {
        result = CreateDebugUtilsMessengerEXT(
            vulkan_instance,
            &dbg_utils_msngr_create_info,
            nullptr,
            &debug_utils_messenger);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger.");
        }
    }

    VkPhysicalDevice physical_device = pick_physical_device();

    if (!physical_device)
    {
        throw std::runtime_error("Invalid physical device");
    }

    //logical_device = create_logical_device(physical_device);
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

    if (device_count == 1)
    {
        return physical_devices[0];
    }

    std::multimap<uint32_t, VkPhysicalDevice> device_candidates;
    for (const auto& device : physical_devices)
    {
        uint32_t score = rate_device_suitability(device);
        device_candidates.insert(std::make_pair(score, device));

        /* Multimap is sorted, so rbegin()->first contains highest score */
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

    if (!check_device_queue_family_support(physical_device))
    {
        throw std::runtime_error("Found physical device, but lacks the required queue family");
    }

    return physical_device;
}

VkDevice liboceanlight::engine::create_logical_device()
{
    VkDevice logical_device {};

    return logical_device;
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

void enable_dbg_utils_msngr(
    std::vector<const char*>& extensions,
    VkDebugUtilsMessengerCreateInfoEXT& dbg_utils_msngr_create_info,
    VkInstanceCreateInfo& instance_create_info)
{
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    dbg_utils_msngr_create_info = populate_dbg_utils_msngr_create_info();
    instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&dbg_utils_msngr_create_info;
}

void enable_vldn_layers(
    VkInstanceCreateInfo& c_info,
    std::vector<const char*>& vldn_layers)
{
    c_info.enabledLayerCount = static_cast<uint32_t>(vldn_layers.size());
    c_info.ppEnabledLayerNames = vldn_layers.data();
}

std::vector<const char*> get_required_instance_extensions()
{
    uint32_t count {0};
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> extensions_vec(extensions, extensions + count);
    return extensions_vec;
}

VkInstanceCreateInfo populate_instance_create_info(VkApplicationInfo* app_info)
{
    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = app_info;
    create_info.pNext = nullptr;
    return create_info;
}

VkApplicationInfo populate_instance_app_info()
{
    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = PROJECT_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(
        PROJECT_VER_MAJOR, PROJECT_VER_MINOR, PROJECT_VER_PATCH);
    app_info.pEngineName = PROJECT_NAME;
    app_info.engineVersion = VK_MAKE_VERSION(
        PROJECT_VER_MAJOR, PROJECT_VER_MINOR, PROJECT_VER_PATCH);
    app_info.apiVersion = VK_API_VERSION_1_3;
    return app_info;
}
