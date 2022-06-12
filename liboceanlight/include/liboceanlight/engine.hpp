#ifndef LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <stdexcept>

void error_callback(int, const char *);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

namespace liboceanlight
{
    class window
    {
        int width {640}, height {480};
        const std::string window_name {"Oceanlight"};
        GLFWwindow *window_pointer {nullptr};

        public:
        window()
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window_pointer = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
            if (window_pointer == NULL)
            {
                throw std::runtime_error("Could not create window");
            }

            glfwSetKeyCallback(window_pointer, key_callback);
        }

        ~window()
        {
            glfwDestroyWindow(window_pointer);
        }

        int should_close()
        {
            return glfwWindowShouldClose(window_pointer);
        }
    };

    class engine
    {
        VkInstance vulkan_instance {nullptr};
        const std::vector<const char*> validation_layers {"VK_LAYER_KHRONOS_validation"};
        const bool validation_layers_enable {true};

    public:
        engine()
        {
            glfwSetErrorCallback(error_callback);

            int rv = glfwInit();
            if (!rv)
            {
                throw std::runtime_error("Failed to initialize glfw");
            }
        }

        ~engine()
        {
            vkDestroyInstance(vulkan_instance, nullptr);
            glfwTerminate();
        }

        void init();
        void instantiate();
        bool check_validation_layer_support();
        void run(liboceanlight::window&);
    };
}

#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */
