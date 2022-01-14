#ifndef ARGS_HPP
#define ARGS_HPP
#include <GLFW/glfw3.h>
struct oceanlight_args_struct
{
    int width, height;
    bool quit_flag;

    oceanlight_args_struct() : quit_flag(false)
    {
        width = 800;
        height = 600;
    }
};

int oceanlight_parse_args(int, char**, struct oceanlight_args_struct*);
GLFWwindow* create_window();
#endif /* ARGS_HPP */
