//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <string>

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

        const char *c_str = klass_name.c_str();

        owner = env->FindClass(c_str);

        if (owner == nullptr)
        {
            env->ExceptionClear();
            env->ThrowNew(env->FindClass("java/lang/ClassNotFound"), c_str);
            throw std::invalid_argument("Unable to find class '" + klass_name + "'");
        }

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