//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <functional>
#include <jvmti.h>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "ZNBKit/jni/signatures/method_signature.hpp"

namespace znb_kit
{
    struct Reference
    {
        void *func_ptr;
        std::vector<std::string> parameters;

        template<typename Func>
        Reference(Func f, const std::vector<std::string> &params)
            :   func_ptr(reinterpret_cast<void *>(f)),
                parameters(params)
        {
        }

        bool has_func() const
        {
            return func_ptr != nullptr;
        }
    };

    class jvmti_object
    {
        jvmtiEnv *jvmti;

        template<typename T = jobject>
        method_signature<T> get_method_signature(JNIEnv *env, const jobject &method);

        template <class T>
        std::vector<method_signature<T>> look_for_method_signatures(JNIEnv *env, const jclass &klass);

        template <class T>
        std::pair<std::vector<JNINativeMethod>, size_t> create_mappings(JNIEnv *env, const klass_signature &klass, const std::unordered_multimap<std::string, Reference> &map);

    public:
        explicit jvmti_object(jvmtiEnv *jvmti): jvmti(jvmti)
        {
            if (jvmti == nullptr)
            {
                throw std::invalid_argument("JVMTI environment cannot be null");
            }
        }

        jvmti_object (const jvmti_object &) = delete;
        jvmti_object &operator=(const jvmti_object &) = delete;

        jvmti_object (jvmti_object &&other) noexcept: jvmti(std::exchange(other.jvmti, nullptr))
        {
        }

        jvmti_object &operator=(jvmti_object &&other) noexcept
        {
            if (this != &other)
            {
                jvmti = std::exchange(other.jvmti, nullptr);
            }

            return *this;
        }

        template <typename... T>
        std::pair<std::vector<JNINativeMethod>, size_t> try_mapping_methods(JNIEnv *env, const klass_signature &klass, const std::unordered_multimap<std::string, Reference> &map);

        void clear_mapped_methods(const std::vector<JNINativeMethod> &vector);

        template <class T>
        method_signature<T> get_method_signature(JNIEnv *env, jclass klass, std::string method_name, std::vector<std::string> parameters);

        [[nodiscard]] jvmtiEnv *get_owner() const
        {
            return jvmti;
        }
    };
}