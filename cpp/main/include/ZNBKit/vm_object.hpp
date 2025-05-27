//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#include <jni.h>
#include <optional>
#include <stdexcept>

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
                throw std::invalid_argument("jvm or jni is null");
            }

            if (jvmti_env != nullptr) {
                jvmti.emplace(jvmti_env);
            }
        }

        vm_object(const vm_object &) = delete;
        vm_object &operator=(const vm_object &) = delete;

        vm_object(vm_object &&other) noexcept :
            jvm(std::exchange(other.jvm, nullptr)),
            jvmti(other.jvmti),
            jni(std::exchange(other.jni, nullptr)),
            version(other.version)
        {
        }

        vm_object &operator=(vm_object &&other) noexcept
        {
            if (this != &other)
            {
                if (jvm != nullptr)
                {
                    jvm->DestroyJavaVM();
                }

                version = other.version;
                jvm = std::exchange(other.jvm, nullptr);
                jni = std::exchange(other.jni, nullptr);
                jvmti = other.jvmti;
            }

            return *this;
        }

        [[nodiscard]] std::optional<std::reference_wrapper<const jvmti_object>> get_jvmti() const
        {
            if (jvmti.has_value())
            {
                return std::ref(jvmti.value());
            }

            return std::nullopt;
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
