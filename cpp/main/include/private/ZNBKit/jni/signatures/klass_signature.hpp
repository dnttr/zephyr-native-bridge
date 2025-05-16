//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <string>

#include "ZNBKit/jni/utils/util.hpp"

class klass_signature
{
    JNIEnv *env;
    jclass owner;

public:
    klass_signature(JNIEnv *env, const std::string &klass_name): env(env)
    {
        if (env == nullptr || klass_name.empty())
        {
            throw std::invalid_argument("JNIEnv or class is invalid");
        }

        const auto klass = util::get_klass(env, klass_name);
        if (klass == nullptr) {
            throw std::runtime_error("Unable to find class " + klass_name);
        }

        owner = reinterpret_cast<jclass>(env->NewGlobalRef(klass));
        env->DeleteLocalRef(klass);
    }

    ~klass_signature()
    {
        if (owner != nullptr && env != nullptr)
        {
            env->DeleteGlobalRef(owner);
            owner = nullptr;
        }
    }

    [[nodiscard]] jclass get_owner() const
    {
        return owner;
    }
};