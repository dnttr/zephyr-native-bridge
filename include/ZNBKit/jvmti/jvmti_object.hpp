//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <iostream>
#include <jvmti.h>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ZNBKit/jvmti/jvmti_factory.hpp"
#include "ZNBKit/jvmti/jvmti_types.hpp"

namespace znb_kit
{
    class jvmti_object
    {
        jvmtiEnv *jvmti;
        JNIEnv *jni;

        static void report_lacking_methods(std::unordered_multimap<std::string, reference>,
                                           std::vector<native_method> &filtered);

        template<typename T>
        struct type_tag { using type = T; };

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
        std::pair<std::vector<native_method>, size_t> create_mappings(
            const klass_signature &klass_signature,
            const std::unordered_multimap<std::string, reference> &map)
        {
            const auto methods = jvmti_factory::look_for_method_signatures<T>(this->jni, this->jvmti, klass_signature);
            auto filtered_mappings = jvmti_factory::map_methods(map, methods);
            size_t size = filtered_mappings.size();

            if (size != map.size())
            {
                report_lacking_methods(map, filtered_mappings);
            }
            return std::make_pair(filtered_mappings, size);
        }

        template <typename... Ts>
        std::pair<std::vector<native_method>, size_t> try_mapping_methods(
            const klass_signature &klass_signature,
            const std::unordered_multimap<std::string, reference> &map)
        {
            std::cout << map.size() << std::endl;
            std::vector<native_method> mapped_methods;
            size_t total = 0;

            (void([&](auto id) {
              using CurrentType = typename decltype(id)::type;
              std::cout << "what" << std::endl;
              auto pair_result = this->create_mappings<CurrentType>(klass_signature, map);
              mapped_methods.insert(mapped_methods.end(), pair_result.first.begin(), pair_result.first.end());
              total += pair_result.second;
          }(type_tag<Ts>{})), ...);


            return {mapped_methods, total};
        }

        static void clear_mapped_methods(const std::vector<JNINativeMethod> &vector);

        [[nodiscard]] jvmtiEnv *get_owner() const { return jvmti; }
    };
}

namespace znb_kit
{
    template std::pair<std::vector<native_method>, size_t> jvmti_object::try_mapping_methods<
        void, jobject, jstring, jint, jlong,
        jboolean, jfloat, jdouble, jbyte, jchar, jshort
    >(const klass_signature &, const std::unordered_multimap<std::string, reference> &);

    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<void>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jobject>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jstring>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jint>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jlong>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jboolean>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jfloat>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jdouble>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jbyte>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jchar>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
    template std::pair<std::vector<native_method>, size_t> jvmti_object::create_mappings<jshort>(
        const klass_signature &, const std::unordered_multimap<std::string, reference> &);
}
