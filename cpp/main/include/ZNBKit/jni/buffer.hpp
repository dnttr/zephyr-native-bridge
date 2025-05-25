//
// Created by Damian Netter on 25/05/2025.
//

#pragma once

#include <jni.h>
#include <ranges>
#include <stdexcept>
#include <utility>

class buffer
{
public:

    static jint set_ptr(JNIEnv *env, const jlongArray &buffer, const uint32_t *ptr, const size_t *size);

    static jint get_ptr(JNIEnv *env, const jlongArray &buffer, std::pair<uint32_t, size_t> &addr);
};
