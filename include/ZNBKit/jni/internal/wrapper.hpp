//
// Created by Damian Netter on 01/06/2025.
//

#pragma once

#include <jni.h>
#include <mutex>
#include <unordered_set>

#include "ZNBKit/debug.hpp"

#define VAR_CHECK(env) \
    if (env == nullptr) { \
        throw std::invalid_argument("Variable 'env' is null"); \
    }

namespace znb_kit
{
    inline thread_local std::unordered_set<jobject> local_refs;

    class global_tracker
    {
        static std::mutex mutex;
        static std::unordered_set<jobject> global_refs;

    public:
        static void add(jobject ref);

        static void remove(jobject ref);

        static size_t count();
    };

    class wrapper
    {
    public:
#define EXCEPT_CHECK(env) \
    if (env->ExceptionCheck()) { \
        env->ExceptionDescribe(); \
        env->ExceptionClear(); \
        throw std::runtime_error("JNI Exception occurred"); \
    }

        static bool check_for_refs();

        static jobject add_local_ref(JNIEnv *env, jobject obj);

        static void remove_local_ref(JNIEnv *env, jobject obj);

        static jobject add_global_ref(JNIEnv *env, jobject obj);

        static void remove_global_ref(JNIEnv *env, jobject obj);

        static jclass search_for_class(JNIEnv *env, const std::string &name);

        static jmethodID get_method(JNIEnv *env, const jclass &klass, const std::string &method_name,
                                    const std::string &signature, bool is_static);

        static jmethodID get_method(JNIEnv *env, const std::string &name, const std::string &method,
                                    const std::string &signature, bool is_static);

        static jobject invoke_object_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                            jmethodID method_id, const std::vector<jvalue> &parameters);

        static jbyte invoke_byte_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                        jmethodID method_id, const std::vector<jvalue> &parameters);

        static jint invoke_int_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                      jmethodID method_id, const std::vector<jvalue> &parameters);

        static jlong invoke_long_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                        jmethodID method_id, const std::vector<jvalue> &parameters);

        static jshort invoke_short_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                          jmethodID method_id, const std::vector<jvalue> &parameters);

        static jfloat invoke_float_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                          jmethodID method_id, const std::vector<jvalue> &parameters);

        static jdouble invoke_double_method(JNIEnv *env, jclass &klass, jobject &instance, jmethodID method_id,
                                            const std::vector<jvalue> &parameters);

        static void invoke_void_method(JNIEnv *env, const jclass &klass, const jobject &instance, jmethodID method_id,
                                       const std::vector<jvalue> &parameters);
    };
}
