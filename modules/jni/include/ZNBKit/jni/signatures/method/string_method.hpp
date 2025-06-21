//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include "ZNBKit/internal/wrapper.hpp"
#include "ZNBKit/jni/signatures/method/object_method.hpp"
#include "ZNBKit/internal/util.hpp"

namespace znb_kit
{
    class string_method final : public object_method
    {
    public:
        string_method(JNIEnv *env, const klass_signature &owner, const std::string &name, const std::string &signature,
            const std::optional<std::vector<std::string>> &parameters, const bool is_static)
            : object_method(env, owner, name, signature, parameters, is_static)
        {
        }

        jstring invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            const auto object = wrapper::invoke_object_method(env, get_owner(), instance, get_identity(), parameters);
            return reinterpret_cast<jstring>(object);
        }

        std::string invoke_and_transform(JNIEnv *env, const jobject &instance, std::vector<jvalue> &parameters)
        {
            return get_string(env, invoke(instance, parameters), true);
        }
    };
}
