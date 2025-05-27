//
// Created by Damian Netter on 25/05/2025.
//

#include "ZNBKit/jni/buffer.hpp"

int znb_kit::buffer::set_ptr_long(JNIEnv *env, const jlongArray &buffer, const jlong *array, const int array_length, const int buffer_offset)
{
    const int buffer_length = get_length(env, buffer, array_length, buffer_offset);
    env->SetLongArrayRegion(buffer, buffer_offset, buffer_length, array);

    return buffer_length;
}

int znb_kit::buffer::get_ptr_long(JNIEnv *env, const jlongArray &buffer, jlong *array, const int array_length, const int buffer_offset)
{
    const int buffer_length = get_length(env, buffer, array_length, buffer_offset);
    env->GetLongArrayRegion(buffer, buffer_offset, buffer_length, array);

    return buffer_length;
}

int znb_kit::buffer::set_ptr_byte(JNIEnv *env, const jbyteArray &buffer, const int8_t *array, const int array_length, const int buffer_offset)
{
    const int buffer_length = get_length(env, buffer, array_length, buffer_offset);
    env->SetByteArrayRegion(buffer, buffer_offset, buffer_length, array);

    return buffer_length;
}

int znb_kit::buffer::get_ptr_byte(JNIEnv *env, const jbyteArray &buffer, int8_t *array, const int array_length, const int buffer_offset)
{
    const int buffer_length = get_length(env, buffer, array_length, buffer_offset);
    env->GetByteArrayRegion(buffer, buffer_offset, buffer_length, array);

    return buffer_length;
}

int znb_kit::buffer::get_length(JNIEnv *env, const jarray &buffer, const int array_length, const int buffer_offset)
{
    if (buffer == nullptr)
    {
        throw std::invalid_argument("buffer is null");
    }

    const auto buffer_length = env->GetArrayLength(buffer);

    if (buffer_length < 0 || buffer_offset >= buffer_length)
    {
        throw std::invalid_argument("buffer size is invalid");
    }

    const auto length = std::min(buffer_length - buffer_offset, array_length);

    return length;
}






