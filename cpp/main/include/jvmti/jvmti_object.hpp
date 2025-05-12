//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <jvmti.h>
#include <map>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "jni/signatures/method/method_signature.hpp"

class jvmti_object
{
    jvmtiEnv *jvmti;

    template<typename T = jobject>
    method_signature<T> get_method_descriptor(JNIEnv *env, jvmtiEnv *jvmti, const jobject &method);

    template<typename T = jobject>
    std::vector<method_signature<T>> gather_method_descriptors(JNIEnv *env, jvmtiEnv *jvmti,
                                                  const std::vector<jobject> &objects);

    template<typename T = jobject>
    std::vector<JNINativeMethod> map_methods(
        const std::map<std::string, typename method_signature<T>::Reference> &map,
        const std::vector<method_signature<T>> &methods, 
        size_t *size);

    void clear_mapped_methods(const std::vector<JNINativeMethod> &vector);

    /*template<typename T = jobject>
    void lookup_methods(std::string name, std::optional<std::vector<std::string>> parameters)
    {
        // Implementation needed
    }*/
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
