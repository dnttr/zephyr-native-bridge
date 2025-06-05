//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <iostream>
#include <jni.h>
#include <string>

#include "ZNBKit/jni/internal/wrapper.hpp"

namespace znb_kit
{
    std::vector<std::string> get_parameters(JNIEnv *env, const local_reference<jobject> &instance);

    bool compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2);

    std::vector<jobject> get_methods(JNIEnv *env, const local_reference<jobject> &instance);
}
