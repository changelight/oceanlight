#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <GLFW/glfw3.h>
int oceanlight_engine_run(GLFWwindow*);
int oceanlight_glfw_init(void);
GLFWwindow *oceanlight_create_window(int, int);
int oceanlight_vk_init();
#endif /* ENGINE_HPP */
