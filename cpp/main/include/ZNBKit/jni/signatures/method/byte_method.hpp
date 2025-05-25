//
// Created by Damian Netter on 13/05/2025.
//

#pragma once

#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class byte_method final : public method_signature<jbyte>
    {
    public:
        using method_signature::method_signature;

        jbyte invoke(const jobject &instance, std::vector<jvalue> &parameters) override {
            if (is_static) {
                return env->CallStaticByteMethod(get_owner(), get_identity(), parameters.data());
            }

            return env->CallByteMethod(instance, get_identity(), parameters.data());
        }
    };
}
