//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include "ZNBKit/internal/wrapper.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class void_method final : public method_signature<void>
    {
    public:
        using method_signature::method_signature;

        void invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            wrapper::invoke_void_method(env, get_owner(), instance, get_identity(), parameters);
        }
    };
}