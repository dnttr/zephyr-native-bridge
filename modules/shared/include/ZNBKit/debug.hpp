//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include <iostream>

inline std::string get_path(const std::string& filePath) {
    const std::string sep = "/cpp/";

    if (const size_t pos = filePath.find(sep); pos != std::string::npos) {
        return filePath.substr(pos + sep.length());
    }

    return "";
}

template<typename T>
 const char* to_c_str(const T& msg) {
    if constexpr (std::is_same_v<T, std::string>) {
        return msg.c_str();
    } else {
        return msg;
    }
}

#ifdef DEBUG
#define debug_print(msg) \
printf("[DEBUG] %s:%d %s\n", get_path(__FILE__).c_str(), __LINE__, to_c_str(msg))
#else
#define debug_print(msg) \
printf("[DEBUG] %s\n", to_c_str(msg))
#endif

#define debug_print_ignore_formatting(msg) \
printf("[DEBUG] %s\n", to_c_str(msg))

#ifdef SHOULD_DISPLAY
#define debug_print_cerr(msg) \
fprintf(stderr, "[DEBUG] %s:%d %s\n", __FILE_NAME__, __LINE__, to_c_str(msg))
#else
#define debug_print_cerr(msg) \
printf("[DEBUG] %s\n", to_c_str(msg))
#endif
