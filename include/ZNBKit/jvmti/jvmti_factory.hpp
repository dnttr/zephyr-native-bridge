//
// Created by Damian Netter on 12/05/2025.
//

#pragma once

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
    /*
     * TODO: refactor further, this class is kinda messy.
     */

    class jvmti_factory {
    public:
        template <class T>
        static std::unique_ptr<method_signature<T>> get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &klass_signature,
                                                                  const jobject &method);
        template <class T>
        static std::unique_ptr<method_signature<T>> get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &klass_signature,
                                                                  std::string method_name,
                                                                  std::vector<std::string> target_params);
        template <class T>
        static std::vector<std::unique_ptr<method_signature<T>>> look_for_method_signatures(
            JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &klass_signature);

        template <typename T>
        static std::vector<JNINativeMethod> map_methods(
        const std::unordered_multimap<std::string, reference> &map,
        const std::vector<std::unique_ptr<method_signature<T>>> &methods);
    };

    template <typename T>
    std::unique_ptr<method_signature<T>> jvmti_factory::get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &klass_signature, const jobject &method) // Parameter changed to const&
    {
        const auto method_id = jni->FromReflectedMethod(method);
        if (!method_id) {
            debug_print("factory::get_method_signature() FromReflectedMethod failed.");
            return nullptr;
        }

        char *name_ptr = nullptr;
        char *signature_ptr = nullptr;

        jvmtiError error = jvmti->GetMethodName(method_id, &name_ptr, &signature_ptr, nullptr);
        if (error != JVMTI_ERROR_NONE)
        {
            char *error_buffer = nullptr;
            jvmti->GetErrorName(error, &error_buffer);
            debug_print("factory::get_method_signature() JVMTI GetMethodName error: " + (error_buffer ? std::string(error_buffer) : "Unknown error"));

            if (error_buffer)
                jvmti->Deallocate(reinterpret_cast<unsigned char *>(error_buffer));
            if (name_ptr)
                jvmti->Deallocate(reinterpret_cast<unsigned char *>(name_ptr));
            if (signature_ptr)
                jvmti->Deallocate(reinterpret_cast<unsigned char *>(signature_ptr));

            return nullptr;
        }

        std::string name_str = name_ptr ? name_ptr : "";
        std::string signature_str = signature_ptr ? signature_ptr : "";

        jint modifiers = 0;

        error = jvmti->GetMethodModifiers(method_id, &modifiers);
         if (error != JVMTI_ERROR_NONE) {
            debug_print("factory::get_method_signature() JVMTI GetMethodModifiers error");
            if (name_ptr) jvmti->Deallocate(reinterpret_cast<unsigned char *>(name_ptr));
            if (signature_ptr) jvmti->Deallocate(reinterpret_cast<unsigned char *>(signature_ptr));
            return nullptr;
        }

        bool is_static = (modifiers & ACC_STATIC) != 0;

        if (signature_ptr) jvmti->Deallocate(reinterpret_cast<unsigned char *>(signature_ptr));
        if (name_ptr) jvmti->Deallocate(reinterpret_cast<unsigned char *>(name_ptr));

        auto params = get_parameters(jni, method);
        auto ptr = std::make_shared<znb_kit::klass_signature>(klass_signature); // klass_signature_ref is now const&


        if constexpr (std::is_same_v<T, jobject>) {
            return std::make_unique<object_method>(jni, ptr, name_str, signature_str, params, is_static);
        } else if constexpr (std::is_same_v<T, jshort>) {
            return std::make_unique<short_method>(jni, ptr, name_str, signature_str, params, is_static);
        } else if constexpr (std::is_same_v<T, jbyte>) {
            return std::make_unique<byte_method>(jni, ptr, name_str, signature_str, params, is_static);
        } else if constexpr (std::is_same_v<T, jint>) {
            return std::make_unique<int_method>(jni, ptr, name_str, signature_str, params, is_static);
        } else if constexpr (std::is_same_v<T, jlong>) {
            return std::make_unique<long_method>(jni, ptr, name_str, signature_str, params, is_static);
        } else if constexpr (std::is_same_v<T, void>) {
            return std::make_unique<void_method>(jni, ptr, name_str, signature_str, params, is_static);
        }

        debug_print("factory::get_method_signature() unhandled type for T in template for method: " + name_str);
        return nullptr;
    }

    template <typename T>
    std::unique_ptr<method_signature<T>> jvmti_factory::get_method_signature(JNIEnv *jni,
        jvmtiEnv *jvmti,
        const klass_signature &klass_signature, // Parameter changed to const&
        std::string method_name,
        const std::vector<std::string> target_params)
    {
        // Assuming get_methods can take const klass_signature& or klass_signature_ref.get_owner() is const-correct
        for (std::vector<jobject> methods_arr = get_methods(jni, klass_signature.get_owner()); jobject &method_obj : methods_arr)
        {
            // Call to the other get_method_signature, which now also takes const klass_signature&
            auto method_desc = jvmti_factory::get_method_signature<T>(jni, jvmti, klass_signature, method_obj);
            if (method_desc && method_name == method_desc->name)
            {
                if (method_desc->parameters.has_value() &&
                    compare_parameters(target_params, method_desc->parameters.value())) {
                    for(const auto cleanup_obj : methods_arr) {
                        if (cleanup_obj != method_obj) jni->DeleteLocalRef(cleanup_obj);
                    }
                    return method_desc;
                }
            }
            jni->DeleteLocalRef(method_obj);
        }
        return nullptr;
    }

    template <typename T>
    std::vector<std::unique_ptr<method_signature<T>>> jvmti_factory::look_for_method_signatures(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &klass_signature_param)
    {
        const auto method_objects = get_methods(jni, klass_signature_param.get_owner());
        std::vector<std::unique_ptr<method_signature<T>>> descriptors;
        descriptors.reserve(method_objects.size());

        for (auto &method_obj : method_objects)
        {
            if (auto method_desc = jvmti_factory::get_method_signature<T>(jni, jvmti, klass_signature_param, method_obj)) {
                descriptors.push_back(std::move(method_desc));
            }
            jni->DeleteLocalRef(method_obj);
        }
        return descriptors;
    }

    template <typename T>
    std::vector<JNINativeMethod> jvmti_factory::map_methods(const std::unordered_multimap<std::string, reference> &map,
        const std::vector<std::unique_ptr<method_signature<T>>> &methods)
    {
        std::vector<JNINativeMethod> result_methods;
        result_methods.reserve(methods.size());

        for (const auto& method_ptr : methods) {
            if (!method_ptr) continue;
            const auto& method = *method_ptr;

            auto range = map.equal_range(method.name);
            for (auto it = range.first; it != range.second; ++it) {
                // Added has_value() check
                if (method.parameters.has_value() && znb_kit::compare_parameters(it->second.parameters, method.parameters.value())) {
                    char* name_dup = strdup(method.name.c_str());
                    char* sig_dup = strdup(method.signature.c_str());
                    if (!name_dup || !sig_dup) {
                        free(name_dup);
                        free(sig_dup);
                        continue;
                    }
                    result_methods.push_back(JNINativeMethod{
                        name_dup,
                        sig_dup,
                        it->second.has_func() ? it->second.func_ptr : nullptr
                    });
                    break;
                }
            }
        }
        return result_methods;
    }

}