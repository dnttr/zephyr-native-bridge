//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <iostream>
#include <jni.h>
#include <string>

namespace Util
{
    inline jmethodID get_method_id(JNIEnv *env, const jclass &klass, const std::string &method_name, const std::string &signature, bool is_static)
    {
        if (is_static)
        {
            return env->GetStaticMethodID(klass, method_name.c_str(), signature.c_str());
        }

        return env->GetMethodID(klass, method_name.c_str(), signature.c_str());
    }

    inline jmethodID get_method_id(JNIEnv *env, const std::string &klass_name, const std::string &method_name, const std::string &signature, bool is_static)
    {
        const auto klass = env->FindClass(klass_name.c_str());

        if (is_static)
        {
            return env->GetStaticMethodID(klass, method_name.c_str(), signature.c_str());
        }

        return env->GetMethodID(klass, method_name.c_str(), signature.c_str());
    }

    inline jclass get_klass(JNIEnv *env, const std::string &name)
    {
        const char *c_str = name.c_str();
        const auto klass = env->FindClass(c_str);

        if (klass == nullptr)
        {
            env->ExceptionClear();
            env->ThrowNew(env->FindClass("java/lang/ClassNotFound"), c_str);
            throw std::invalid_argument("Unable to find class '" + name + "'");
        }

        return klass;
    }

    inline std::string get_string(JNIEnv *env, const jstring &string, const bool release = true) {
        const char *key = env->GetStringUTFChars(string, nullptr);

        if (key == nullptr)
        {
            std::cerr << "get_string >> " << "Failed to get UTF string" << std::endl;
            return {};
        }

        std::string str(key);

        if (release)
        {
            env->ReleaseStringUTFChars(string, key);
        }

        return str;
    }

    inline void delete_references(JNIEnv *env, const std::vector<jobject> &references)
    {
        for (const auto &reference : references)
        {
            env->DeleteLocalRef(reference);
        }
    }

    inline std::vector<std::string> get_parameters(JNIEnv *env, const jobject &method)
    {
        if (method == nullptr)
        {
            std::cerr << "get_parameters >> " << "Unable to obtain parameters as provided method object is null" << std::endl;
            return {};
        }

        const auto getParameterTypes_method_id  = get_method_id(env, "java/lang/reflect/Method", "getParameterTypes", "()[Ljava/lang/Class;", false);

        if (getParameterTypes_method_id == nullptr)
        {
            return {};
        }

        const auto getTypeName_method_id = get_method_id(env, "java/lang/Class", "getTypeName", "()Ljava/lang/String;", false);

        if (getTypeName_method_id == nullptr)
        {
            return {};
        }

        const auto array = reinterpret_cast<jobjectArray>(env->CallObjectMethod(method, getParameterTypes_method_id));
        const auto array_size = env->GetArrayLength(array);

        if (array == nullptr)
        {
            std::cerr << "get_parameters >> " << "Unable to get array, as it is null" << std::endl;

            return {};
        }

        std::vector<std::string> methods(array_size);

        for (int i = 0; i < array_size; ++i)
        {
            const auto element = env->GetObjectArrayElement(array, i);
            const auto jstr = reinterpret_cast<jstring>(env->CallObjectMethod(element, getTypeName_method_id));
            const auto key = get_string(env, jstr);
            methods[i] = key;

            env->DeleteLocalRef(jstr);
            env->DeleteLocalRef(element);
        }

        env->DeleteLocalRef(array);

        return methods;
    }

    inline bool compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2) {
        if (v1.size() != v2.size()) {
            return false;
        }

        std::unordered_set set(v1.begin(), v1.end());

        if (set.size() != v1.size()) {
            return false;
        }

        return std::ranges::any_of(v2, [&set](const std::string &s) {
            return set.contains(s);
        });
    }
}
