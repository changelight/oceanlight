#include <iostream>
#include <liboceanlight/util.hpp>
#include <config.h>
#include "args.hpp"

int main(int argc, char **argv)
{
    parse_args(argc, argv);
    return EXIT_SUCCESS;
}
