//
// Created by Damian Netter on 13/05/2025.
//

#pragma once

#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class long_method final : public method_signature<jlong>
    {
    public:
        using method_signature::method_signature;

        jlong invoke(const jobject &instance, std::vector<jvalue> &parameters) override {
            if (is_static) {
                return env->CallStaticLongMethod(get_owner(), get_identity(), parameters.data());
            }

            return env->CallLongMethod(instance, get_identity(), parameters.data());
        }
    };
}
