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

        auto localRef = util::get_klass(env, klass_name);
        if (localRef == nullptr) {
            throw std::runtime_error("Unable to find class " + klass_name);
        }
        // Convert local reference to global reference
        owner = reinterpret_cast<jclass>(env->NewGlobalRef(localRef));
        // Delete the local reference since we now have a global reference
        env->DeleteLocalRef(localRef);
    }

    ~klass_signature()
    {
        if (owner != nullptr && env != nullptr)
        {
            env->DeleteGlobalRef(owner); // Delete global ref, not local
            owner = nullptr;
        }
    }

    [[nodiscard]] jclass get_owner() const
    {
        return owner;
    }
};