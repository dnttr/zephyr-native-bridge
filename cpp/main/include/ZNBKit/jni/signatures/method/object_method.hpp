//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class object_method : public method_signature<jobject>
    {
    public:
        using method_signature::method_signature;

        jobject invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            if (is_static) {
                return env->CallStaticObjectMethod(get_owner(), get_identity(), parameters.data());
            }

            return env->CallObjectMethod(instance, get_identity(), parameters.data());
        }
    };
}