#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLFWwindow* create_window()
{   
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);

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