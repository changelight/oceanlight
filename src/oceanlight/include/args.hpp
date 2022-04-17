#ifndef ARGS_HPP
#define ARGS_HPP
#include <iostream>
#include <GLFW/glfw3.h>
struct oceanlight_args
{
    int width, height;
    bool exit_flag;

    oceanlight_args() : width(800), height(600), exit_flag(false) {}
};

void oceanlight_parse_args(int, char**, struct oceanlight_args*);
GLFWwindow* create_window();
#endif /* ARGS_HPP */
