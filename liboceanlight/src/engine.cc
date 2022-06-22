#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/engine.hpp>
#include <liboceanlight/util.hpp>
#include <config.h>

void liboceanlight::engine::instantiate()
{
    VkResult result;
    VkApplicationInfo app_info = populate_vulkan_application_info();
    VkInstanceCreateInfo create_info = populate_vulkan_create_info(&app_info);

    uint32_t required_extension_count {0};
    const char** initial_required_extensions = glfwGetRequiredInstanceExtensions(&required_extension_count);
    std::vector<const char*> required_extensions(initial_required_extensions, initial_required_extensions + required_extension_count);
    const std::vector<const char*> validation_layers {"VK_LAYER_KHRONOS_validation"};

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info;
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
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    if (validation_layers_enable)
    {
        result = CreateDebugUtilsMessengerEXT(vulkan_instance, &debug_utils_messenger_create_info, nullptr, &debug_utils_messenger);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
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
