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
                //add?
                wrapper::add_global_ref(vm->get_env(), object);
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

            object = wrapper::add_global_ref(env, obj);
            wrapper::remove_local_ref(env, obj);
        }

        instance(const instance &other) = delete;
        instance& operator=(const instance &other) = delete;

        instance(instance &&other) noexcept: object(std::exchange(other.object, nullptr)), vm(other.vm)
        {
        }

        auto operator=(instance &&other) noexcept -> instance &
        {
            if (this == &other)
                return *this;

            cleanup();
            object = std::exchange(other.object, nullptr);
            vm = other.vm;

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