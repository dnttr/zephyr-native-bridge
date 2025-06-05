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
        global_reference<jobject> object;
        vm_object *vm{nullptr};

    public:

        explicit instance(vm_object *vm, const void_method &method_signature, const std::vector<local_value_reference> &parameters): vm(vm)
        {
            if (vm == nullptr)
            {
                throw std::invalid_argument("vm cannot be null");
            }

            JNIEnv* jni = vm->get_env();

            const auto obj = wrapper::new_object(jni, method_signature.get_owner()->get_owner(), method_signature.get_identity(), parameters);
            object = wrapper::change_reference_policy<local_reference<jobject>>(jni, wrapper::jni_reference_policy::GLOBAL, obj);
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

            object = std::exchange(other.object, nullptr);
            vm = other.vm;

            return *this;
        }

        [[nodiscard]] const auto &get_owner() const
        {
            return object;
        }
    };
}