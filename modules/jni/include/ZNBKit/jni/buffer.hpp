//
// Created by Damian Netter on 25/05/2025.
//

#pragma once

#include <jni.h>
#include <ranges>

namespace znb_kit
{
    class buffer
    {
        static int get_length(JNIEnv *env, const jarray &buffer, int array_length, int buffer_offset = 0);
    public:

        static int set_ptr_long(JNIEnv *env, const jlongArray &buffer, const jlong *array, int array_length, int buffer_offset = 0);

        static int get_ptr_long(JNIEnv *env, const jlongArray &buffer, jlong *array, int array_length, int buffer_offset = 0);

        static int set_ptr_byte(JNIEnv *env, const jbyteArray &buffer, const int8_t *array, int array_length, int buffer_offset = 0);

        static int get_ptr_byte(JNIEnv *env, const jbyteArray &buffer, int8_t *array, int array_length, int buffer_offset = 0);
    };
}