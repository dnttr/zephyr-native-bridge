//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include "jni/signatures/method/method_signature.hpp"

class void_method final : public method_signature<void>
{
public:
    using method_signature::method_signature;

    void invoke(const klass_signature &instance) override
    {
        if (is_static)
        {
            env->CallStaticVoidMethod(get_owner(), identity);
        } else
        {
            env->CallVoidMethod(instance.get_owner(), identity);
        }
    }
};
