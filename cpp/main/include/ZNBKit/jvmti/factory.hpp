//
// Created by Damian Netter on 30/05/2025.
//

#pragma once
#include <memory>

#include "jvmti_object.hpp"

namespace znb_kit
{
    class factory {
        template <class T>
        std::unique_ptr<method_signature<T>> get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, jclass klass,
                                                                  const jobject &method);
        template <class T>
        std::unique_ptr<method_signature<T>> get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, jclass klass,
                                                                  std::string method_name,
                                                                  std::vector<std::string> parameters);
        template <class T>
        std::vector<std::unique_ptr<method_signature<T>>> look_for_method_signatures(
            JNIEnv *jni, jvmtiEnv *jvmti, const jclass &klass);
    };
}
