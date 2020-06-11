#ifndef UTILS_HPP
#define UTILS_HPP

#include "tinyformat.h"

#include <string>

template<typename... Args>
void log(const char* fmt, const Args&... args)
{
    static const std::string prefix = "dinput_proxy: ";
    static const std::string postfix = "\n";
    const std::string msg = tfm::format(fmt, args...);
    ::OutputDebugString((prefix + msg + postfix).c_str());
}

#endif // !UTILS_HPP
