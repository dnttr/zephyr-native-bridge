//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#include <jni.h>
#include <jvmti.h>

class vm_object {

    JavaVM *vm;
    jvmtiEnv *jvmti;
    JNIEnv *env;

public:

    vm_object(JavaVM *vm, jvmtiEnv *jvmti, JNIEnv *env):
        vm(vm),
        jvmti(jvmti),
        env(env)
    {
        if (vm == nullptr || jvmti == nullptr || env == nullptr)
        {
            throw std::invalid_argument("vm, jvmti or env is null");
        }
    }

    [[nodiscard]] jvmtiEnv *get_jvmti() const
    {
        return jvmti;
    }

    [[nodiscard]] JavaVM *get_vm() const
    {
        return vm;
    }

    [[nodiscard]] JNIEnv *get_env() const
    {
        JNIEnv *env = nullptr;

        if (const jint res = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_21); res == JNI_EDETACHED)
        {
            JavaVMAttachArgs vm_attach_args;
            vm_attach_args.version = JNI_VERSION_21;

            if (vm->AttachCurrentThread(reinterpret_cast<void **>(&env), &vm_attach_args) != JNI_OK)
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
        if (vm != nullptr)
        {
            vm->DestroyJavaVM();
            vm = nullptr;
        }
    }
};