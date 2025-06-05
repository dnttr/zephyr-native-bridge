//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include "ZNBKit/jni/internal/wrapper.hpp"
#include "ZNBKit/jni/signatures/method/object_method.hpp"

namespace znb_kit
{
    class string_method final : public object_method
    {
    public:
        string_method(JNIEnv *env, std::shared_ptr<klass_signature> owner, const std::string &name, const std::string &signature,
            const std::optional<std::vector<std::string>> &parameters, const bool is_static)
            : object_method(env, std::move(owner), name, signature, parameters, is_static)
        {
        }

        jstring invoke(const jobject &instance, std::vector<local_value_reference> &parameters) override
        {
            const auto object = wrapper::invoke_string_method(env, get_owner(), instance, get_identity(), parameters);
            return object.get();
        }

        std::string invoke_and_transform(JNIEnv *env, const jobject &instance, const std::vector<local_value_reference> &parameters) const
        {
            const auto object = wrapper::invoke_string_method(env, get_owner(), instance, get_identity(), parameters);
            return wrapper::get_string(env, object);
        }
    };
}