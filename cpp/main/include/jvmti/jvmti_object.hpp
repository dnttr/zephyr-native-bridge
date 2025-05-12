//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <jvmti.h>
#include <map>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "../jni/signatures/method_signature.hpp"

// Move the Reference struct outside the class
struct Reference
{
    std::optional<void *> func_ptr;
    std::vector<std::string> parameters;

    Reference(void *func_ptr, const std::vector<std::string> &params): func_ptr(func_ptr), parameters(params)
    {
    }
};

class jvmti_object
{
    jvmtiEnv *jvmti;
    
    template<typename T = jobject>
    method_signature<T> get_method_descriptor(JNIEnv *env, const jobject &method);

    template <class T>
    std::vector<method_signature<T>> gather_method_descriptors(JNIEnv *env, const jclass &klass);
    template <class T>
    method_signature<T> find_method(JNIEnv *env, jclass klass, std::string method_name,
                                    std::vector<std::string> parameters);

    template<typename T = jobject>
    std::vector<JNINativeMethod> map_methods(
        const std::map<std::string, Reference> &map,
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

    // Optional: Add type alias for backward compatibility
    // using Reference = ::Reference;
};
