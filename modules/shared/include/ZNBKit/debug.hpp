//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include <iostream>

inline std::string get_path(const std::string &filePath)
{
    const std::string sep = "/cpp/";

    if (const size_t pos = filePath.find(sep); pos != std::string::npos)
    {
        return filePath.substr(pos + sep.length());
    }

    return "";
}

template <typename T>
const char *to_c_str(const T &msg)
{
    if constexpr (std::is_same_v<T, std::string>)
    {
        return msg.c_str();
    }
    else
    {
        return msg;
    }
}

#define debug_print(msg) do { } while (0)
#define debug_print_ignore_formatting(msg) do { } while (0)
#define debug_print_cerr(msg) do { } while (0)