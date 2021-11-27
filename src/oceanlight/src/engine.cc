#include <iostream>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLFWwindow* create_window(int width, int height)
{   
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(width, height, "Vulkan Window", nullptr, nullptr);

    return window;
}

void glfw_set_callbacks(GLFWwindow *window)
{
    glfwSetKeyCallback(window, key_callback);
}

int glfw_init()
{
    glfwInit();
    return 1;
}

int vk_init()
{
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::cout << extension_count << " extensions supported\n";

    return 1;
}

int run(GLFWwindow *window)
{
    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    return 1;
}