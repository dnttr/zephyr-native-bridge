//
// Created by Damian Netter on 13/05/2025.
//

#pragma once

#include "ZNBKit/jni/internal/wrapper.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class long_method final : public method_signature<jlong>
    {
    public:
        using method_signature::method_signature;

        jlong invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            return wrapper::invoke_long_method(env, get_owner(), instance, get_identity(), parameters);
        }
    };
}
