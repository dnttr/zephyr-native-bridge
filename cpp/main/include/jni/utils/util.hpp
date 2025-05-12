//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <iostream>
#include <jni.h>
#include <string>

namespace util
{
    jmethodID get_method_id(JNIEnv *env, const jclass &klass, const std::string &method_name, const std::string &signature, bool is_static);

    jmethodID get_method_id(JNIEnv *env, const std::string &klass_name, const std::string &method_name, const std::string &signature, bool is_static);

    jclass get_klass(JNIEnv *env, const std::string &name);

    std::string get_string(JNIEnv *env, const jstring &string, bool release = true);

    void delete_references(JNIEnv *env, const std::vector<jobject> &references);

    std::vector<std::string> get_parameters(JNIEnv *env, const jobject &method);

    bool compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2);
}
