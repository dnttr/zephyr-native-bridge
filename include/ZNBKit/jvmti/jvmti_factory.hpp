//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include <cstring>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include <jni.h>
#include <jvmti.h>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/signatures/method/byte_method.hpp"
#include "ZNBKit/jni/signatures/method/int_method.hpp"
#include "ZNBKit/jni/signatures/method/long_method.hpp"
#include "ZNBKit/jni/signatures/method/object_method.hpp"
#include "ZNBKit/jni/signatures/method/short_method.hpp"
#include "ZNBKit/jni/signatures/method/void_method.hpp"
#include "ZNBKit/jvmti/jvmti_types.hpp"

#ifndef ACC_STATIC
#define ACC_STATIC 0x0008
#endif

namespace znb_kit
{
    //TODO: change klass_signature to another name, its confusing.
    class jvmti_factory
    {
        template <typename T>
        static std::unique_ptr<method_signature<T>> create_method_instance(
            JNIEnv *jni,
            const klass_signature &klass_signature,
            const std::string &name,
            const std::string &signature,
            const std::optional<std::vector<std::string>> &params,
            bool is_static);
    public:
        template <class T>
        static std::unique_ptr<method_signature<T>> get_method_signature(
            JNIEnv *jni,
            jvmtiEnv *jvmti,
            const klass_signature &klass_signature,
            const jobject &method);

        template <class T>
        static std::unique_ptr<method_signature<T>> get_method_signature(
            JNIEnv *jni,
            jvmtiEnv *jvmti,
            const klass_signature &klass_signature,
            std::string method_name,
            std::vector<std::string> target_params);

        template <class T>
        static std::vector<std::unique_ptr<method_signature<T>>> look_for_method_signatures(
            JNIEnv *jni,
            jvmtiEnv *jvmti,
            const klass_signature &klass_signature);

        template <typename T>
        static std::vector<JNINativeMethod> map_methods(
            const std::unordered_multimap<std::string, reference> &map,
            const std::vector<std::unique_ptr<method_signature<T>>> &methods);
    };
}
