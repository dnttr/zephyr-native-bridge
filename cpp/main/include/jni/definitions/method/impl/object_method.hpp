//
// Created by Damian Netter on 11/05/2025.
//
#pragma once

#include "jni/definitions/method/method_signature.hpp"

class object_method final : public method_signature<jobject>
{
public:
    using method_signature::method_signature;

    jobject invoke(const klass_signature &instance) override {
        if (is_static) {
            return env->CallStaticObjectMethod(get_owner(), get_identity());
        }

        return env->CallObjectMethod(instance.get_owner(), get_identity());
    }
};
