//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <string>

#include "jni/utils/util.hpp"

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

        owner = Util::get_klass(env, klass_name);
        env->NewLocalRef(owner);
    }

    ~klass_signature()
    {
        if (owner != nullptr && env != nullptr)
        {
            env->DeleteLocalRef(owner);
            owner = nullptr;
        }
    }

    [[nodiscard]] jclass get_owner() const
    {
        return owner;
    }
};