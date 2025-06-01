//
// Created by Damian Netter on 13/05/2025.
//

#pragma once

#include "ZNBKit/jni/internal/wrapper.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class short_method final : public method_signature<jshort>
    {
        using method_signature::method_signature;

        short invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            return wrapper::invoke_short_method(env, get_owner(), instance, get_identity(), parameters);
        }
    };
}