//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <string>

class KlassProxy
{
    JNIEnv *env;
    jclass klass;
public:

    KlassProxy(JNIEnv *env, const std::string &klass_name): env(env)
    {
        if (env == nullptr || klass_name.empty())
        {
            throw std::invalid_argument("JNIEnv or class is invalid");
        }

        const char *c_str = klass_name.c_str();

        klass = env->FindClass(c_str);

        if (klass == nullptr)
        {
            env->ExceptionClear();
            env->ThrowNew(env->FindClass("java/lang/ClassNotFound"), c_str);
            throw std::invalid_argument("Unable to find class '" + klass_name + "'");
        }

        env->NewLocalRef(klass);
    }

    ~KlassProxy()
    {
        if (klass != nullptr && env != nullptr)
        {
            env->DeleteLocalRef(klass);
            klass = nullptr;
        }
    }

    [[nodiscard]] jclass get_klass() const
    {
        return klass;
    }
};
