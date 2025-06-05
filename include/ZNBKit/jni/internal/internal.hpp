//
// Created by Damian Netter on 03/06/2025.
//

#pragma once

#include <jni.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#define VAR_CHECK(param) \
    if (param == nullptr) { \
        throw std::invalid_argument("Variable '" #param "' is null"); \
    }

#define VAR_CONTENT_CHECK(param) \
    if (param.empty()) { \
        throw std::invalid_argument("Variable '" #param "' is empty"); \
    }

#define EXCEPT_CHECK(jni) \
    if (jni->ExceptionCheck()) { \
        jni->ExceptionDescribe(); \
        jni->ExceptionClear(); \
        throw std::runtime_error("JNI Exception occurred"); \
    }

namespace internal
{
    enum class types
    {
        BOOLEAN,
        BYTE,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        CHAR,

        OBJECT
    };

    struct ref_info {
        std::string file;
        int line;
        std::string method;
        std::string details;
    };

    class tracker_manager
    {
    public:
        static std::unordered_map<std::string, size_t> tracked_native_classes;

        static thread_local std::unordered_map<jobject, ref_info> local_ref_sources;
        static thread_local std::unordered_set<jobject> local_refs;

        static jobject add_local_ref(JNIEnv *jni, const jobject &obj,
                                     const std::string &file = __FILE__, int line = __LINE__,
                                     const std::string &method = __builtin_FUNCTION());

        static jobject add_global_ref(JNIEnv *jni, const jobject &obj,
                                      const std::string &file = __FILE__, int line = __LINE__,
                                      const std::string &method = __builtin_FUNCTION());

        static void remove_local_ref(JNIEnv *jni, const jobject &obj);

        static void remove_global_ref(JNIEnv *jni, const jobject &obj);

        static void check_for_corruption();

        static void dump_local_refs();
    };

    class global_tracker
    {
        static std::mutex mutex;

    public:
        static std::unordered_map<jobject, ref_info> global_ref_sources;
        static std::unordered_set<jobject> global_refs;

        static void add(const jobject &ref, const std::string &file = "", int line = 0, const std::string &method = "");

        static void remove(const jobject &ref);

        static size_t count();

        static void dump_refs();
    };
}
