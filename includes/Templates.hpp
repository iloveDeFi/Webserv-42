#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <sstream>
#include <string>

template <typename T>
std::string to_string(const T &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

#endif