#ifndef UTIL_HPP
#define UTIL_HPP
#include <iostream>
#include <string>
namespace lo
{
    template <typename T>
    void print(std::ostream &out, const T& t)
    {
        out << t << std::endl;
    };

    template <typename T, typename... Args>
    void print(std::ostream &out, const T& t, const Args... args)
    {
        out << t << std::endl;
        print(out, args...);
    };

    std::string version_string(void);
}
#endif /* UTIL_HPP */
