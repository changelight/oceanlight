#ifndef LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <liboceanlight/util.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <config.h>

void error_callback(int, const char*);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

namespace liboceanlight
{
    class window
    {
        int width {640}, height {480};
        const std::string window_name {"Oceanlight"};
        GLFWwindow* window_pointer {nullptr};

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

    struct queue_family_indices_struct
    {
        std::optional<uint32_t> graphics_family;
    };

    class engine
    {
        const bool validation_layers_enable {true};
        VkDebugUtilsMessengerEXT debug_utils_messenger;

    public:
        VkInstance vulkan_instance {nullptr};

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
            if (debug_utils_messenger)
            {
                DestroyDebugUtilsMessengerEXT(vulkan_instance, debug_utils_messenger, nullptr);
            }

            vkDestroyInstance(vulkan_instance, nullptr);
            glfwTerminate();
        }

        void init();
        VkPhysicalDevice pick_physical_device();
        uint32_t rate_device_suitability(VkPhysicalDevice);
        bool check_device_queue_family_support(VkPhysicalDevice);
        liboceanlight::queue_family_indices_struct find_queue_families(VkPhysicalDevice);
        void run(liboceanlight::window&);
    };
}
#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */
