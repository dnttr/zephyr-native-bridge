//
// Created by Damian Netter on 12/05/2025.
//

#include "vm_object.hpp"


JNIEnv * vm_object::get_env() const
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
