#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <iostream>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <config.h>
void error_callback(int, const char *);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

struct oceanlight_engine_instance
{
    void init();
    void run(GLFWwindow*);

    oceanlight_engine_instance()
    {
        glfwSetErrorCallback(error_callback);

        int rv = glfwInit();
        if (!rv)
        {
            throw std::runtime_error("Failed to initialize glfw");
        }
    }

    ~oceanlight_engine_instance()
    {
        glfwTerminate();
    }
};

struct oceanlight_window
{
    int width, height;
    GLFWwindow *window_pointer;

    oceanlight_window(int w, int h) : width(w), height(h)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window_pointer = glfwCreateWindow(width, height, "Oceanlight", nullptr, nullptr);

        if (window_pointer == NULL)
        {
            throw std::runtime_error("Could not create window");
        }

        glfwSetKeyCallback(window_pointer, key_callback);
    }

    ~oceanlight_window()
    {
        glfwDestroyWindow(window_pointer);
    }
};

void oceanlight_init(void);
int oceanlight_vulkan_init();
int oceanlight_engine_run(GLFWwindow *);
#endif /* ENGINE_HPP */
