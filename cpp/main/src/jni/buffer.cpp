//
// Created by Damian Netter on 25/05/2025.
//

#include "ZNBKit/jni/buffer.hpp"

jint buffer::set_ptr(JNIEnv *env, const jlongArray &buffer, const uint32_t *ptr, const size_t *size)
{
    if (buffer == nullptr)
    {
        throw std::invalid_argument("buffer is null");
    }

    if (env->GetArrayLength(buffer) < 2)
    {
        throw std::invalid_argument("buffer size is invalid");
    }

    const auto ptr_addr = reinterpret_cast<jlong>(ptr);
    const auto size_addr = reinterpret_cast<jlong>(size);

    env->SetLongArrayRegion(buffer, 0, 1, &ptr_addr);
    env->SetLongArrayRegion(buffer, 1, 1, &size_addr);

    return 0;
}

jint buffer::get_ptr(JNIEnv *env, const jlongArray &buffer, std::pair<uint32_t, size_t> &addr)
{
    if (buffer == nullptr)
    {
        throw std::invalid_argument("buffer is null");
    }

    if (env->GetArrayLength(buffer) < 2)
    {
        throw std::invalid_argument("buffer size is invalid");
    }

    jlong ptr_addr = 0;
    jlong size_addr = 0;

    env->GetLongArrayRegion(buffer, 0, 1, &ptr_addr);
    env->GetLongArrayRegion(buffer, 1, 1, &size_addr);

    if (size_addr == 0)
    {
        return 1;
    }

    const auto ptr = reinterpret_cast<const uint32_t *>(ptr_addr);
    const auto size = reinterpret_cast<const size_t *>(size_addr);

    if (ptr == nullptr || size == nullptr)
    {
        throw std::runtime_error("Invalid pointer or size retrieved from buffer");
    }

    addr.first = *ptr;
    addr.second = *size;

    return 0;
}


