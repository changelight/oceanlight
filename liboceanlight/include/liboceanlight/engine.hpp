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

struct queue_family_indices_struct
{
    std::optional<uint32_t> graphics_family;
};

VkApplicationInfo populate_instance_app_info(void);
VkInstanceCreateInfo populate_instance_create_info(VkApplicationInfo&);
std::vector<const char*> get_required_instance_extensions(void);

bool check_vldn_layer_support(
    const std::vector<const char*>& validation_layers);

void enable_vldn_layers(
    VkInstanceCreateInfo&,
    std::vector<const char*>&);

void enable_dbg_utils_msngr(
    std::vector<const char*>&,
    VkDebugUtilsMessengerCreateInfoEXT&,
    VkInstanceCreateInfo&);

VkPhysicalDevice pick_physical_device(
    VkInstance&,
    queue_family_indices_struct&);

bool check_device_queue_family_support(
    VkPhysicalDevice&,
    queue_family_indices_struct&);

void find_queue_families(VkPhysicalDevice&, queue_family_indices_struct&);
uint32_t rate_device_suitability(const VkPhysicalDevice&);
VkDevice create_logical_device(VkPhysicalDevice&, queue_family_indices_struct&);

void error_callback(int, const char*);
void key_callback(GLFWwindow*, int, int, int, int);

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

    class engine
    {
        const bool validation_layers_enabled {true};
        VkDebugUtilsMessengerEXT debug_utils_messenger;

    public:
        VkInstance vulkan_instance {nullptr};
        VkDevice logical_device {nullptr};

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
                DestroyDebugUtilsMessengerEXT(
                    vulkan_instance,
                    debug_utils_messenger,
                    nullptr);
            }

            vkDestroyDevice(logical_device, nullptr);
            vkDestroyInstance(vulkan_instance, nullptr);
            glfwTerminate();
        }

        void init();
        void run(liboceanlight::window&);
    };
}
#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */
