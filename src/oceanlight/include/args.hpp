#ifndef ARGS_H
#define ARGS_H
#include <GLFW/glfw3.h>
struct args_struct
{
    int width, height;
    bool quit_flag;

    args_struct() : quit_flag(false)
    {
        width = 800;
        height = 600;
    }
};

int parse_args(int, char**, struct args_struct*);
GLFWwindow* create_window();
#endif /* ARGS_H */