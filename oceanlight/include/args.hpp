#ifndef OCEANLIGHT_ARGS_HPP_INCLUDED
#define OCEANLIGHT_ARGS_HPP_INCLUDED
#include <iostream>
namespace oceanlight
{
    struct args
    {
        int width, height;
        bool exit_flag;

        args() : width(640), height(480), exit_flag(false) {};
    };

    void parse_args(int, char **, struct oceanlight::args&);
}
#endif /* OCEANLIGHT_ARGS_HPP_INCLUDED */
