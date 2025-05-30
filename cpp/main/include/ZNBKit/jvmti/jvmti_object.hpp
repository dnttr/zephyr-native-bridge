//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <jvmti.h>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "factory.hpp"
#include "jvmti_types.hpp"

namespace znb_kit
{
    class jvmti_object
    {
        jvmtiEnv *jvmti;
        JNIEnv *jni;

    public:
        explicit jvmti_object(JNIEnv *new_jni, jvmtiEnv *new_jvmti): jvmti(new_jvmti), jni(new_jni)
        {
            if (jni == nullptr)
            {
                throw std::invalid_argument("JNI environment cannot be null");
            }
            if (jvmti == nullptr)
            {
                throw std::invalid_argument("JVMTI environment cannot be null");
            }
        }

        jvmti_object(const jvmti_object &) = delete;
        jvmti_object &operator=(const jvmti_object &) = delete;

        jvmti_object(jvmti_object &&other) noexcept: jni(std::exchange(other.jni, nullptr)),
                                                     jvmti(std::exchange(other.jvmti, nullptr))
        {
        }

        jvmti_object &operator=(jvmti_object &&other) noexcept
        {
            if (this != &other)
            {
                jni = std::exchange(other.jni, nullptr);
                jvmti = std::exchange(other.jvmti, nullptr);
            }
            return *this;
        }

        template <typename T>
        std::pair<std::vector<JNINativeMethod>, size_t> create_mappings(
            const jclass &klass,
            const std::unordered_multimap<std::string, Reference> &map)
        {
            const auto methods = factory::look_for_method_signatures<T>(this->jni, this->jvmti, klass);
            auto filtered_mappings = factory::map_methods(map, methods);
            size_t size = filtered_mappings.size();

            if (size != map.size())
            {
                //TODO: report missing methods, too lazy to do it now
            }
            return std::make_pair(filtered_mappings, size);
        }

        template <typename... Ts>
        std::pair<std::vector<JNINativeMethod>, size_t> try_mapping_methods(
            const jclass &klass,
            const std::unordered_multimap<std::string, Reference> &map)
        {
            std::vector<JNINativeMethod> mapped_methods;
            size_t total = 0;

            ([&]
            {
                auto pair_result = create_mappings<Ts>(klass, map);
                mapped_methods.insert(mapped_methods.end(), pair_result.first.begin(), pair_result.first.end());
                total += pair_result.second;
            }(), ...);

            return {mapped_methods, total};
        }

        static void clear_mapped_methods(const std::vector<JNINativeMethod> &vector);

        [[nodiscard]] jvmtiEnv *get_owner() const { return jvmti; }
    };
}

namespace znb_kit
{
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::try_mapping_methods<
        void, jobject, jstring, jint, jlong,
        jboolean, jfloat, jdouble, jbyte, jchar, jshort
    >(const jclass &, const std::unordered_multimap<std::string, Reference> &);

    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<void>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jobject>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jstring>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jint>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jlong>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jboolean>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jfloat>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jdouble>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jbyte>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jchar>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jshort>(
        const jclass &, const std::unordered_multimap<std::string, Reference> &);
}
