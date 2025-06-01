//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <iostream>
#include <jni.h>
#include <set>
#include <string>

namespace znb_kit
{
    extern std::set<jobject> g_refs;
    extern std::set<jobject> l_refs;

    inline jobject trackNewGlobalRef(JNIEnv* env, jobject obj) {
        const auto ref = env->NewGlobalRef(obj);
        if (ref) {
            g_refs.insert(ref);
            std::cout << "Global ref created: " << ref << " (count: " << g_refs.size() << ")\n";
        }
        return ref;
    }

    inline jobject trackNewLocalRef(JNIEnv* env, jobject obj) {
        const auto ref = env->NewLocalRef(obj);
        if (ref) {
            l_refs.insert(ref);
            std::cout << "Local ref created: " << ref << " (count: " << l_refs.size() << ")\n";
        }
        return ref;
    }

    inline void trackDeleteGlobalRef(JNIEnv* env, jobject obj) {
        if (obj) {
            g_refs.erase(obj);
            std::cout << "Global ref deleted: " << obj << " (count: " << g_refs.size() << ")\n";
            env->DeleteGlobalRef(obj);
        }
    }

    inline void trackDeleteLocalRef(JNIEnv* env, jobject obj) {
        if (obj) {
            l_refs.erase(obj);
            std::cout << "Local ref deleted: " << obj << " (count: " << l_refs.size() << ")\n";
            env->DeleteLocalRef(obj);
        }
    }

    inline void checkRemainingRefs() {
        if (!g_refs.empty()) {
            std::cerr << "WARNING: " << g_refs.size() << " global refs leaked!\n";
        } else
        {
            std::cerr << "All global refs cleaned up successfully.\n";
        }
    }

    jmethodID get_method_id(JNIEnv *env, const jclass &klass, const std::string &method_name, const std::string &signature, bool is_static);

    jmethodID get_method_id(JNIEnv *env, const std::string &klass_name, const std::string &method_name, const std::string &signature, bool is_static);

    jclass get_klass(JNIEnv *env, const std::string &name);

    std::string get_string(JNIEnv *env, const jstring &string, bool release = true);

    void delete_references(JNIEnv *env, const std::vector<jobject> &references);

    std::vector<std::string> get_parameters(JNIEnv *env, const jobject &method);

    bool compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2);

    std::vector<jobject> get_methods(JNIEnv *env, const jclass &klass);
}
