#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <liboceanlight/engine.hpp>
#include <config.h>

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

    uint32_t glfw_extension_count {0};
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    create_info.enabledExtensionCount = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    VkResult result = vkCreateInstance(&create_info, nullptr, &vulkan_instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    uint32_t vulkan_extension_count {0};
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, nullptr);

    std::vector<VkExtensionProperties> vulkan_extensions(vulkan_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkan_extension_count, vulkan_extensions.data());

    std::cout << extension_count << " Vulkan extensions supported:\n";
    for (const auto& extension : vulkan_extensions)
    {
        std::cout << extension.extensionName << "\n";
    }

    bool validation_layer_support = check_validation_layer_support();
    if (validation_layers_enable && !validation_layer_support)
    {
        throw std::runtime_error("Requested validation layers, but none available.");
    }
    else if (validation_layers_enable && validation_layer_support)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
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
