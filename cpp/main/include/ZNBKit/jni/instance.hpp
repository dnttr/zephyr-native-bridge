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
        jobject object{nullptr};
        vm_object *vm{nullptr};

        void cleanup()
        {
            if (object != nullptr && vm != nullptr)
            {
                vm->get_env()->DeleteGlobalRef(object);
                object = nullptr;
            }
        }

    public:

        explicit instance(vm_object *vm, const void_method &method_signature, const std::vector<jvalue> &parameters): vm(vm)
        {
            if (vm == nullptr)
            {
                throw std::invalid_argument("vm cannot be null");
            }

            JNIEnv* env = vm->get_env();

            const auto obj = env->NewObjectA(method_signature.get_owner(), method_signature.get_identity(), parameters.data());

            if (obj == nullptr) {
                env->ExceptionClear();
                throw std::runtime_error("Unable to create new instance");
            }

            object = env->NewGlobalRef(obj);
            env->DeleteLocalRef(obj);
        }

        instance(const instance &other) = delete;
        instance& operator=(const instance &other) = delete;

        instance(instance &&other) noexcept: object(other.object), vm(other.vm)
        {
            other.object = nullptr;
        }

        auto operator=(instance &&other) noexcept -> instance &
        {
            if (this == &other)
                return *this;

            cleanup();
            object = other.object;
            vm = other.vm;
            other.object = nullptr;

            return *this;
        }

        ~instance()
        {
            cleanup();
        }

        [[nodiscard]] jobject get_owner() const
        {
            return object;
        }
    };
}