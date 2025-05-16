//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class void_method final : public method_signature<void>
    {
    public:
        using method_signature::method_signature;

        void invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            if (is_static)
            {
                env->CallStaticVoidMethod(get_owner(), identity, parameters.data());
            } else
            {
                env->CallVoidMethod(instance, identity);
            }
        }
    };
}