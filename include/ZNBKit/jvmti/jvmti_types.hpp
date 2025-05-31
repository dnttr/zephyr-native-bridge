//
// Created by Damian Netter on 30/05/2025.
//

#pragma once

#include <string>
#include <vector>

namespace znb_kit
{
    struct native_method
    {
        std::vector<char> name_buffer;
        std::vector<char> signature_buffer;

        JNINativeMethod jni_method{};

        native_method(const std::string &name, const std::string &signature, void *func_ptr)
        {
            name_buffer.assign(name.begin(), name.end());
            name_buffer.push_back('\0');

            signature_buffer.assign(signature.begin(), signature.end());
            signature_buffer.push_back('\0');

            jni_method.name = name_buffer.data();
            jni_method.signature = signature_buffer.data();
            jni_method.fnPtr = func_ptr;
        }

        [[nodiscard]] const JNINativeMethod &get_jni_method() const
        {
            return jni_method;
        }
    };

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
