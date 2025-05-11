//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#include <jni.h>
#include <jvmti.h>

class vm_object {

    JavaVM *jvm;
    std::optional<jvmtiEnv *> jvmti;
    JNIEnv *jni;

    int version;
public:

    vm_object(const int version, JavaVM *jvm, const std::optional<jvmtiEnv *> jvmti, JNIEnv *jni):
        version(version),
        jvm(jvm),
        jvmti(jvmti),
        jni(jni)
    {
        if (jvm == nullptr || jni == nullptr)
        {
            throw std::invalid_argument("vm, jvmti or env is null");
        }
    }

    [[nodiscard]] jvmtiEnv *get_jvmti() const
    {
        return jvmti.value_or(nullptr);
    }

    [[nodiscard]] JavaVM *get_vm() const
    {
        return jvm;
    }

    [[nodiscard]] JNIEnv *get_env() const
    {
        JNIEnv *env = nullptr;

        if (const jint res = jvm->GetEnv(reinterpret_cast<void **>(&env), version); res == JNI_EDETACHED)
        {
            JavaVMAttachArgs vm_attach_args;
            vm_attach_args.version = version;

            if (jvm->AttachCurrentThread(reinterpret_cast<void **>(&env), &vm_attach_args) != JNI_OK)
            {
                throw std::runtime_error("failed to attach vm thread");
            }
        } else if (res != JNI_OK)
        {
            throw std::runtime_error("failed to get JNIEnv");
        }

        return env;
    }

    ~vm_object()
    {
        if (jvm != nullptr)
        {
            jvm->DestroyJavaVM();
            jvm = nullptr;
        }
    }
};