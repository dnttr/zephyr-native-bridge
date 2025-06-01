//
// Created by Damian Netter on 01/06/2025.
//

#pragma once

#include <jni.h>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include "ZNBKit/debug.hpp"

#define VAR_CHECK(env) \
    if (env == nullptr) { \
        throw std::invalid_argument("Variable 'env' is null"); \
    }

#define EXCEPT_CHECK(env) \
    if (env->ExceptionCheck()) { \
        env->ExceptionDescribe(); \
        env->ExceptionClear(); \
        throw std::runtime_error("JNI Exception occurred"); \
}

namespace znb_kit
{
    struct ref_info {
        std::string file;
        int line;
        std::string method;
        std::string details;
    };

    inline thread_local std::unordered_set<jobject> local_refs;
    inline thread_local std::unordered_map<jobject, ref_info> local_ref_sources;

    class global_tracker
    {
        static std::mutex mutex;
        static std::unordered_set<jobject> global_refs;
        static std::unordered_map<jobject, ref_info> global_ref_sources;

    public:
        static void add(const jobject &ref, const std::string &file = "", int line = 0, const std::string &method = "");

        static void remove(const jobject &ref);

        static size_t count();

        static void dump_refs();
    };

    class wrapper
    {
    public:
        static void check_for_refs();

        static jobject add_local_ref(JNIEnv *env, const jobject &obj,
                           const std::string &file = __FILE__, int line = __LINE__,
                           const std::string &method = __builtin_FUNCTION());

        static jobject add_global_ref(JNIEnv *env, const jobject &obj,
                                    const std::string &file = __FILE__, int line = __LINE__,
                                    const std::string &method = __builtin_FUNCTION());

        static void remove_local_ref(JNIEnv *env, const jobject &obj);

        static void remove_global_ref(JNIEnv *env, const jobject &obj);

        static void dump_local_refs();

        static jclass search_for_class(JNIEnv *env, const std::string &name,
                                     const std::string &caller_file = __builtin_FILE(),
                                     int caller_line = __builtin_LINE(),
                                     const std::string &caller_function = __builtin_FUNCTION());

        static jmethodID get_method(JNIEnv *env, const jclass &klass, const std::string &method_name,
                                    const std::string &signature, bool is_static);

        static jmethodID get_method(JNIEnv *env, const std::string &name, const std::string &method,
                                    const std::string &signature, bool is_static);

        static jobject invoke_object_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                            const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jbyte invoke_byte_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jint invoke_int_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                      const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jlong invoke_long_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jshort invoke_short_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jfloat invoke_float_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jdouble invoke_double_method(JNIEnv *env, const jclass &klass, const jobject &instance, const jmethodID &method_id,
                                            const std::vector<jvalue> &parameters);

        static void invoke_void_method(JNIEnv *env, const jclass &klass, const jobject &instance, const jmethodID &method_id,
                                       const std::vector<jvalue> &parameters);
    };
}