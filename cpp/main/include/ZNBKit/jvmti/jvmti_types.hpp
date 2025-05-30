//
// Created by Damian Netter on 30/05/2025.
//

#pragma once

#include <string>
#include <vector>

namespace znb_kit
{
    struct reference
    {
        void *func_ptr;
        std::vector<std::string> parameters;

        template <typename Func>
        reference(Func f, const std::vector<std::string> &params)
            : func_ptr(reinterpret_cast<void *>(f)),
              parameters(params)
        {
        }

        [[nodiscard]] bool has_func() const
        {
            return func_ptr != nullptr;
        }
    };
}
