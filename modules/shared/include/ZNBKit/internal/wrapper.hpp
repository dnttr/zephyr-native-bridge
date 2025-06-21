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
    enum mapping
    {
        VOID,
        STRING,
        INT,
        BYTE,
        LONG,
        SHORT,
        FLOAT,
        DOUBLE,
        OBJECT,
        BOOLEAN,

        STRING_ARRAY,
        INT_ARRAY,
        BYTE_ARRAY,
        LONG_ARRAY,
        SHORT_ARRAY,
        FLOAT_ARRAY,
        DOUBLE_ARRAY,
        OBJECT_ARRAY,
        BOOLEAN_ARRAY,
    };

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
        void *fn_ptr;

        jni_native_method(const std::string &name_str, const std::string &sig_str, void *func_ptr_val)
            : fn_ptr(func_ptr_val)
        {
            name_buffer.assign(name_str.begin(), name_str.end());
            name_buffer.push_back('\0');

            signature_buffer.assign(sig_str.begin(), sig_str.end());
            signature_buffer.push_back('\0');
        }

        jni_native_method() : fn_ptr(nullptr) {}
    };


    struct jni_bridge_reference
    {
        void *func_ptr;
        std::vector<std::string> parameters;

        template <typename Func>
        [[deprecated("Deprecated constructor. Use either jni_bridge_reference(Func) or jni_bridge_reference(Func, const std::vector<mapping>&) instead.")]]
        jni_bridge_reference(Func f, const std::vector<std::string> &params)
            : func_ptr(reinterpret_cast<void *>(f)),
              parameters(params)
        {
        }

        template <typename Func>
        jni_bridge_reference(Func f, const std::vector<mapping> &params) : func_ptr(reinterpret_cast<void *>(f))
        {
            parameters.reserve(params.size());

            for (const auto &param : params)
            {
                switch (param)
                {
                case VOID:
                    throw std::invalid_argument("JNI bridge reference cannot have VOID type as a parameter");
                case STRING:
                    parameters.emplace_back("java.lang.String");
                    break;
                case INT:
                    parameters.emplace_back("int");
                    break;
                case BYTE:
                    parameters.emplace_back("byte");
                    break;
                case LONG:
                    parameters.emplace_back("long");
                    break;
                case SHORT:
                    parameters.emplace_back("short");
                    break;
                case FLOAT:
                    parameters.emplace_back("float");
                    break;
                case DOUBLE:
                    parameters.emplace_back("double");
                    break;
                case OBJECT:
                    parameters.emplace_back("object");
                    break;
                case BOOLEAN:
                    parameters.emplace_back("boolean");
                    break;
                case STRING_ARRAY:
                    throw std::invalid_argument("Bridge does not support STRING_ARRAY type as a parameter as of now");
                case INT_ARRAY:
                    parameters.emplace_back("int[]");
                    break;
                case BYTE_ARRAY:
                    parameters.emplace_back("byte[]");
                    break;
                case LONG_ARRAY:
                    parameters.emplace_back("long[]");
                    break;
                case SHORT_ARRAY:
                    parameters.emplace_back("short[]");
                    break;
                case FLOAT_ARRAY:
                    parameters.emplace_back("float[]");
                    break;
                case DOUBLE_ARRAY:
                    parameters.emplace_back("double[]");
                    break;
                case OBJECT_ARRAY:
                    throw std::invalid_argument("Bridge does not support OBJECT_ARRAY type as a parameter as of now");
                case BOOLEAN_ARRAY:
                    break;
                }
            }
        }

        template <typename Func>
        explicit jni_bridge_reference(Func f) : func_ptr(reinterpret_cast<void *>(f))
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
    public:
        static std::mutex mutex;
        static std::unordered_set<jobject> global_refs;
        static std::unordered_map<jobject, ref_info> global_ref_sources;

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

        static void cleanup_all_refs(JNIEnv* jni);

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

        static void unregister_natives(JNIEnv *jni, const std::string &klass_name);
    };
}