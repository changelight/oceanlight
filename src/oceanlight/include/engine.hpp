#ifndef ENGINE_H
#define ENGINE_H
#include <GLFW/glfw3.h>
int run(GLFWwindow*);
int glfw_init(void);
void glfw_set_callbacks(GLFWwindow*);
int vk_init();
#endif /* ENGINE_H */