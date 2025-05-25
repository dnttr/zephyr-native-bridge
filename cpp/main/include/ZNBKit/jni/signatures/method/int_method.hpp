//
// Created by Damian Netter on 13/05/2025.
//

#pragma once

#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class int_method final : public method_signature<jint>
    {
    public:
        using method_signature::method_signature;

        jint invoke(const jobject &instance, std::vector<jvalue> &parameters) override {
            if (is_static) {
                return env->CallStaticIntMethod(get_owner(), get_identity(), parameters.data());
            }

            return env->CallIntMethod(instance, get_identity(), parameters.data());
        }
    };
}
