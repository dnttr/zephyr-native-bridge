//
// Created by Damian Netter on 13/05/2025.
//

#pragma once

#include "ZNBKit/jni/signatures/method_signature.hpp"

class short_method final : public method_signature<jshort>
{
    using method_signature::method_signature;

    short invoke(const jobject &instance, std::vector<jvalue> &parameters) override
    {
        if (is_static) {
            return env->CallStaticShortMethod(get_owner(), get_identity(), parameters.data());
        }

        return env->CallShortMethod(instance, get_identity(), parameters.data());
    }
};
