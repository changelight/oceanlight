#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <GLFW/glfw3.h>
int run(GLFWwindow*);
int glfw_init(void);
void glfw_set_callbacks(GLFWwindow*);
GLFWwindow *create_window(int, int);
int vk_init();
#endif /* ENGINE_HPP */
