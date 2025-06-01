//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <iostream>
#include <jni.h>
#include <string>

namespace znb_kit
{
    std::string get_string(JNIEnv *env, const jstring &string, bool release = true);

    void delete_references(JNIEnv *env, const std::vector<jobject> &references);

    std::vector<std::string> get_parameters(JNIEnv *env, const jobject &method);

    bool compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2);

    std::vector<jobject> get_methods(JNIEnv *env, const jobject &instance);
}
