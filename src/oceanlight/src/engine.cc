#include <iostream>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <liboceanlight/error.hpp>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void error_callback(int code, const char* description)
{
    lo_print_error(code, description);
}

int oceanlight_glfw_init()
{
    glfwSetErrorCallback(error_callback);

    int rv = glfwInit();

    if (rv == GLFW_FALSE)
    {
        return 0;
    }

    return 1;
}

int oceanlight_vulkan_init()
{
    VkInstance instance;
    
    return 1;
}

GLFWwindow* oceanlight_create_window(int width, int height)
{   
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(width, height, "Vulkan Window", nullptr, nullptr);

    glfwSetKeyCallback(window, key_callback);

    return window;
}

int oceanlight_vk_init()
{
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::cout << extension_count << " Vulkan extensions supported" << std::endl;

    return 1;
}

int oceanlight_engine_run(GLFWwindow *window)
{
    while(!glfwWindowShouldClose(window))
    {
        //glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    return 1;
}
