#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
//#include <glm/mat4x4.hpp>
//#include <glm/vec4.hpp>
#include <liboceanlight/engine.hpp>
#include <config.h>

void liboceanlight::engine::init()
{
    uint32_t extension_count {0};
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Oceanlight";
    app_info.applicationVersion = VK_MAKE_VERSION(PROJECT_VER_MAJOR, PROJECT_VER_MINOR, PROJECT_VER_PATCH);
    app_info.pEngineName = "Changelight";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = 0;

    uint32_t glfw_extension_count {0};
    const char **glfw_extensions;
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
    for (const auto &extension : vulkan_extensions)
    {
        std::cout << extension.extensionName << "\n";
    }
}

void liboceanlight::engine::run(liboceanlight::window &window)
{
    while(!window.should_close())
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
