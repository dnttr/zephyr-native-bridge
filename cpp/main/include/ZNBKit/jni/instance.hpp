//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include <jni.h>
#include <stdexcept>

#include "ZNBKit/vm_object.hpp"
#include "ZNBKit/jni/signatures/method/void_method.hpp"

namespace znb_kit
{
    class instance {
        jobject object;
        vm_object *vm;

    public:

        explicit instance(vm_object *vm, const void_method &method_signature, const std::vector<jvalue> &parameters): vm(vm)
        {
            JNIEnv* env = vm->get_env();

            const auto obj = env->NewObjectA(method_signature.get_owner(),method_signature.get_identity(), parameters.data());

            if (obj == nullptr) {
                env->ExceptionClear();
                env->ThrowNew(env->FindClass("java/lang/InstantiationException"),
                              "Unable to create new instance");
                throw std::runtime_error("Unable to create new instance");
            }

            object = env->NewGlobalRef(obj);
            env->DeleteLocalRef(obj);
        }

        ~instance()
        {
            if (object != nullptr)
            {
                vm->get_env()->DeleteGlobalRef(object);
                object = nullptr;
            }
        }

        [[nodiscard]] jobject get_owner() const
        {
            return object;
        }
    };
}