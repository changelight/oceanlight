#ifndef LIBOCEANLIGHT_GLFW_ERR_CALLBACK_HPP_INCLUDED
#define LIBOCEANLIGHT_GLFW_ERR_CALLBACK_HPP_INCLUDED
#include <GLFW/glfw3.h>
void lol_glfw_error_callback(int, const char*);
void lol_glfw_key_callback(GLFWwindow*, int, int, int, int);
void lol_glfw_scroll_callback(GLFWwindow*, double, double);
void lol_glfw_cursor_pos_callback(GLFWwindow*, double, double);
void lol_glfw_framebuffer_size_callback(GLFWwindow*, int, int);
#endif /* LIBOCEANLIGHT_GLFW_ERR_CALLBACK_HPP_INCLUDED */
