//
// Created by Damian Netter on 01/06/2025.
//

#pragma once

#include <jni.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

namespace znb_kit
{

    struct ref_info {
        std::string file;
        int line;
        std::string method;
        std::string details;
    };

    struct jni_native_method
    {
        std::vector<char> name_buffer;
        std::vector<char> signature_buffer;

        JNINativeMethod jni_method{};

        jni_native_method(const std::string &name, const std::string &signature, void *func_ptr)
        {
            name_buffer.assign(name.begin(), name.end());
            name_buffer.push_back('\0');

            signature_buffer.assign(signature.begin(), signature.end());
            signature_buffer.push_back('\0');

            jni_method.name = name_buffer.data();
            jni_method.signature = signature_buffer.data();
            jni_method.fnPtr = func_ptr;
        }
    };

    struct jni_bridge_reference
    {
        void *func_ptr;
        std::vector<std::string> parameters;

        template <typename Func>
        jni_bridge_reference(Func f, const std::vector<std::string> &params)
            : func_ptr(reinterpret_cast<void *>(f)),
              parameters(params)
        {
        }

        [[nodiscard]] bool has_func() const
        {
            return func_ptr != nullptr;
        }
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
        static std::unordered_map<std::string, size_t> tracked_native_classes;

    public:
        static void check_for_corruption();

        static jobject add_local_ref(JNIEnv *jni, const jobject &obj,
                           const std::string &file = __FILE__, int line = __LINE__,
                           const std::string &method = __builtin_FUNCTION());

        static jobject add_global_ref(JNIEnv *jni, const jobject &obj,
                                    const std::string &file = __FILE__, int line = __LINE__,
                                    const std::string &method = __builtin_FUNCTION());

        static void remove_local_ref(JNIEnv *jni, const jobject &obj);

        static void remove_global_ref(JNIEnv *jni, const jobject &obj);

        static void dump_local_refs();

        static jclass search_for_class(JNIEnv *jni, const std::string &name,
                                     const std::string &caller_file = __builtin_FILE(),
                                     int caller_line = __builtin_LINE(),
                                     const std::string &caller_function = __builtin_FUNCTION());

        static jmethodID get_method(JNIEnv *jni, const jclass &klass, const std::string &method_name,
                                    const std::string &signature, bool is_static);

        static jmethodID get_method(JNIEnv *jni, const std::string &name, const std::string &method,
                                    const std::string &signature, bool is_static);

        static jobject invoke_object_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                            const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jbyte invoke_byte_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jint invoke_int_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                      const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jlong invoke_long_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jshort invoke_short_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jfloat invoke_float_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters);

        static jdouble invoke_double_method(JNIEnv *jni, const jclass &klass, const jobject &instance, const jmethodID &method_id,
                                            const std::vector<jvalue> &parameters);

        static void invoke_void_method(JNIEnv *jni, const jclass &klass, const jobject &instance, const jmethodID &method_id,
                                       const std::vector<jvalue> &parameters);

        static void register_natives(JNIEnv *jni, const std::string &klass_name, const jclass &klass, const std::vector<jni_native_method> &methods);

        static void unregister_natives(JNIEnv *jni, const std::string &klass_name, const jclass &klass);

        static std::string obtain_klass_name(JNIEnv *jni, const jclass &klass);
    };
}