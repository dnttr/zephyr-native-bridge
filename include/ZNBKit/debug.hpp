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

#ifdef DEBUG
#define debug_print(msg) \
(std::cout << "[DEBUG] " << get_path(__FILE__) << ":" << __LINE__ << " " << msg << std::endl)
#else
#define debug_print(msg) do {} while (0)
#endif

#ifdef DEBUG
#define debug_print_ignore_formatting(msg) \
(std::cout << "[DEBUG] " << msg << std::endl)
#else
#define debug_print_ignore_formatting(msg) do {} while (0)
#endif

#ifdef DEBUG
#define debug_print_cerr(msg) \
(std::cerr << "[DEBUG] " << get_path(__FILE__) << ":" << __LINE__ << " " << msg << std::endl)
#else
#define debug_print_cerr(msg) do {} while (0)
#endif

