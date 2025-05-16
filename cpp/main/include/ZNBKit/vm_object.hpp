//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#include <jni.h>
#include <optional>
#include <stdexcept>

#include "debug.hpp"
#include "ZNBKit/jvmti/jvmti_object.hpp"

namespace znb_kit
{
    class vm_object {
        JavaVM *jvm;
        std::optional<jvmti_object> jvmti;
        JNIEnv *jni;

        int version;
    public:
        vm_object(const int version, JavaVM *jvm, jvmtiEnv *jvmti_env, JNIEnv *jni):
            version(version),
            jvm(jvm),
            jni(jni)
        {
            if (jvm == nullptr || jni == nullptr)
            {
                throw std::invalid_argument("vm or env is null");
            }

            if (jvmti_env != nullptr) {
                jvmti.emplace(jvmti_env);
            }
        }

        [[nodiscard]] jvmti_object* get_jvmti() const
        {
            return jvmti.has_value() ? const_cast<jvmti_object*>(&jvmti.value()) : nullptr;
        }

        [[nodiscard]] JavaVM *get_owner() const
        {
            return jvm;
        }

        [[nodiscard]] JNIEnv *get_env() const;

        ~vm_object()
        {
            if (jvm != nullptr)
            {
                jvm->DestroyJavaVM();
                jvm = nullptr;
            }
        }
    };
}
