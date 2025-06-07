//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

#include <jni.h>
#include <jvmti.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "ZNBKit/jni/signatures/method_signature.hpp"

#ifndef ACC_STATIC
#define ACC_STATIC 0x0008
#endif

namespace znb_kit
{
    class jvmti_factory
    {
        template <typename T>
        static std::unique_ptr<method_signature<T>> create_method_instance(
            JNIEnv *jni,
            const klass_signature &owner_ks,
            const std::string &name,
            const std::string &signature,
            const std::optional<std::vector<std::string>> &params,
            bool is_static);

    public:
        template <class T>
        static std::unique_ptr<method_signature<T>> jvmti_factory::get_method_signature(
            JNIEnv *jni, jvmtiEnv *jvmti, global_reference<jclass> owner,
            const global_reference<jobject> &method);

        template <class T>
        static std::unique_ptr<method_signature<T>> get_method_signature(
            JNIEnv *jni, jvmtiEnv *jvmti, global_reference<jobject> owner,
            const std::string &method_name,
            const std::vector<std::string> &target_params);

        template <class T>
        static std::vector<std::unique_ptr<method_signature<T>>> look_for_method_signatures(
            JNIEnv *jni,
            jvmtiEnv *jvmti,
            const klass_signature &owner_ks);

        template <typename T>
        static std::vector<jni_native_method> map_methods(
            const std::unordered_multimap<std::string, jni_bridge_reference> &map,
            const std::vector<std::unique_ptr<method_signature<T>>> &methods);
    };
}
