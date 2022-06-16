#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/engine.hpp>
#include <config.h>

VKAPI_ATTR VkBool32 VKAPI_CALL liboceanlight::engine::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    std::cerr << "VALIDATION LAYER: " << callback_data->pMessage << "\n";
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        throw std::runtime_error("Vulkan validation layer detected a fatal error");
    }
    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT liboceanlight::engine::create_debug_messenger()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = liboceanlight::engine::debug_callback;
    create_info.pUserData = nullptr; // Optional

    return create_info;
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

void liboceanlight::engine::DestroyDebugUtilsMessengerEXT(
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

std::vector<const char*> liboceanlight::engine::get_required_extensions()
{
    uint32_t glfw_initial_extension_count {0};
    const char** glfw_initial_extensions = glfwGetRequiredInstanceExtensions(&glfw_initial_extension_count);
    std::vector<const char*> glfw_full_extensions(glfw_initial_extensions, glfw_initial_extensions + glfw_initial_extension_count);

    return glfw_full_extensions;
}

bool liboceanlight::engine::check_validation_layer_support()
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

void liboceanlight::engine::instantiate()
{
    uint32_t extension_count {0};
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = PROJECT_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR, PROJECT_VER_MINOR, PROJECT_VER_PATCH);
    app_info.pEngineName = PROJECT_NAME;
    app_info.engineVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR, PROJECT_VER_MINOR, PROJECT_VER_PATCH);
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    std::vector<const char*> required_extensions = get_required_extensions();

    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info;
    if (validation_layers_enable)
    {
        bool validation_layer_support = check_validation_layer_support();
        if (!validation_layer_support)
        {
            throw std::runtime_error("Validation layers requested, but none available.");
        }

        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        messenger_create_info = create_debug_messenger();
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    for (const auto& i : required_extensions)
    {
        std::cout << "REQUIRED EXTENSIONS: " << i << "\n";
    }

    uint32_t vulkan_extension_count {0};
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, nullptr);

    std::vector<VkExtensionProperties> vulkan_extensions(vulkan_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, vulkan_extensions.data());

    std::cout << "AVAILABLE EXTENSIONS (" << vulkan_extension_count << "): \n";
    for (const auto& extension : vulkan_extensions)
    {
        std::cout << extension.extensionName << "\n";
    }

    VkResult result = vkCreateInstance(&create_info, nullptr, &vulkan_instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    if (CreateDebugUtilsMessengerEXT(vulkan_instance, &messenger_create_info, nullptr, &debug_messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
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
