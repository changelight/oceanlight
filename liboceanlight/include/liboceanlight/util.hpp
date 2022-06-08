#ifndef LIBOCEANLIGHT_UTIL_HPP_INCLUDED
#define LIBOCEANLIGHT_UTIL_HPP_INCLUDED
#include <iostream>
#include <string>
namespace liboceanlight
{
    /* Avoid using this in hotpaths for now since I don't know if the
    parameter unpacking is done via single-stepforwarding or recursion.
    If it is recursion, it will be slower. Also because we flush the stream. */
    template <typename... Args>
    std::ostream& generic_print(std::ostream& out, Args&&... args)
    {
        ((out << args << "\n"), ... ) << std::endl;
        return out;
    }

    std::string version_string(void);
}
#endif /* LIBOCEANLIGHT_UTIL_HPP_INCLUDED */
