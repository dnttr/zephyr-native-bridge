//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include "ZNBKit/jni/internal/wrapper.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    class object_method : public method_signature<jobject>
    {
    public:
        using method_signature::method_signature;

        jobject invoke(const jobject &instance, std::vector<jvalue> &parameters) override
        {
            return wrapper::invoke_object_method(env, get_owner(), instance, get_identity(), parameters);
        }
    };
}