//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include "ZNBKit/jni/signatures/method/object_method.hpp"

namespace znb_kit
{
    class string_method final : public object_method
    {
    public:
        string_method(JNIEnv *env, klass_signature *owner, std::string &name, std::string &signature,
            const std::optional<std::vector<std::string>> &parameters, const bool is_static)
            : object_method(env, owner, name, signature, parameters, is_static)
        {
        }

        jstring invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            jobject object;

            if (is_static) {
                object = env->CallStaticObjectMethod(get_owner(), get_identity());
            } else
            {
                object = env->CallObjectMethod(instance, get_identity());
            }

            return reinterpret_cast<jstring>(object);
        }

        std::string invoke_and_transform(JNIEnv *env, const jobject &instance, std::vector<jvalue> &parameters)
        {
            return get_string(env, invoke(instance, parameters), true);
        }
    };
}