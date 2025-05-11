//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <jvmti.h>
#include <stdexcept>

class jvmti_object {

    jvmtiEnv *jvmti;
public:

    explicit jvmti_object(jvmtiEnv *jvmti): jvmti(jvmti)
    {
        if (jvmti == nullptr)
        {
            throw std::runtime_error("JVMTI is null");
        }
    }

    [[nodiscard]] jvmtiEnv *get_owner() const
    {
        return jvmti;
    }
};
