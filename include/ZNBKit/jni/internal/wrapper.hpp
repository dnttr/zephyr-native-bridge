//
// Created by Damian Netter on 01/06/2025.
//

#pragma once

#include <jni.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "ZNBKit/jni/internal/internal.hpp"

template <typename T>
using jni_local_ref = std::unique_ptr<T, internal::jni_reference_deleter<T, internal::jni_local_policy>>;

template <typename T>
using jni_global_ref = std::unique_ptr<T, internal::jni_reference_deleter<T, internal::jni_global_policy>>;

using jni_local_value_ref = std::unique_ptr<jvalue, internal::jni_reference_value_deleter<internal::jni_local_policy>>;
using jni_global_value_ref = std::unique_ptr<jvalue, internal::jni_reference_value_deleter<internal::jni_global_policy>>;

namespace znb_kit
{
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

    class wrapper
    {
        /*static constexpr std::string_view java_lang_Class = "java/lang/Class";
        static constexpr std::string_view java_lang_reflect_Method = "java/lang/reflect/Method";

        static constexpr std::string_view getParameterTypes_name = "getParameterTypes";
        static constexpr std::string_view getParameterTypes_signature = "()[Ljava/lang/Class;";

        static constexpr std::string_view getDeclaredMethods_name = "getDeclaredMethods";
        static constexpr std::string_view getDeclaredMethods_signature = "()[Ljava/lang/reflect/Method;";

        static constexpr std::string_view getTypeName_name = "getTypeName";
        static constexpr std::string_view getTypeName_signature = "()Ljava/lang/String;";*/

        template <typename T>
        static jni_local_ref<T> make_local(JNIEnv *env, T ref)
        {
            return jni_local_ref<T>(ref, {env});
        }

        template <typename T>
        static jni_global_ref<T> make_global(JNIEnv *env, T ref)
        {
            return jni_global_ref<T>(ref, {env});
        }

    public:
        enum class jni_reference_policy
        {
            LOCAL,
            GLOBAL
        };

        static jobject add_local_ref(JNIEnv *jni, const jobject &obj, const std::string &file, int line,
                                     const std::string &method, bool needed_new_ref);
        static void remove_local_ref(JNIEnv *jni, const jobject &obj);

        static jobject add_global_ref(JNIEnv *jni, const jobject &obj, const std::string &file, int line,
                                      const std::string &method, bool needed_new_ref);
        static void remove_global_ref(JNIEnv *jni, const jobject &obj);

        template <class T>
        auto change_reference_policy(JNIEnv *jni, jni_reference_policy new_policy, const T &reference);

        static void check_for_corruption();

        static jni_local_ref<jclass> search_for_class(JNIEnv *jni, const std::string &name,
                                                      const std::string &caller_file = __builtin_FILE(),
                                                      int caller_line = __builtin_LINE(),
                                                      const std::string &caller_function = __builtin_FUNCTION());

        static jmethodID get_method(JNIEnv *jni, const jni_local_ref<jclass> &klass, const std::string &method_name,
                                    const std::string &signature, bool is_static);

        static jmethodID get_method(JNIEnv *jni, const std::string &name, const std::string &method,
                                    const std::string &signature, bool is_static);

        static jni_local_ref<jobject> invoke_object_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                                           const jni_local_ref<jobject> &instance,
                                                           const jmethodID &method_id,
                                                           const std::vector<jni_local_value_ref> &parameters);

        static std::pair<jni_local_ref<jobjectArray>, size_t> invoke_object_array_method(
            JNIEnv *jni, const jni_local_ref<jclass> &klass,
            const jni_local_ref<jobject> &instance,
            const jmethodID &method_id,
            const std::vector<jni_local_value_ref> &parameters);

        static jni_local_ref<jstring> invoke_string_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                                           const jni_local_ref<jobject> &instance,
                                                           const jmethodID &method_id,
                                                           const std::vector<jni_local_value_ref> &parameters);

        static jbyte invoke_byte_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                        const jni_local_ref<jobject> &instance,
                                        const jmethodID &method_id, const std::vector<jni_local_value_ref> &parameters);

        static jint invoke_int_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                      const jni_local_ref<jobject> &instance,
                                      const jmethodID &method_id, const std::vector<jni_local_value_ref> &parameters);

        static jlong invoke_long_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                        const jni_local_ref<jobject> &instance,
                                        const jmethodID &method_id, const std::vector<jni_local_value_ref> &parameters);

        static jshort invoke_short_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                          const jni_local_ref<jobject> &instance,
                                          const jmethodID &method_id, const std::vector<jni_local_value_ref> &parameters);

        static jfloat invoke_float_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                          const jni_local_ref<jobject> &instance,
                                          const jmethodID &method_id, const std::vector<jni_local_value_ref> &parameters);

        static jdouble invoke_double_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                            const jni_local_ref<jobject> &instance, const jmethodID &method_id,
                                            const std::vector<jni_local_value_ref> &parameters);

        static void invoke_void_method(JNIEnv *jni, const jni_local_ref<jclass> &klass,
                                       const jni_local_ref<jobject> &instance, const jmethodID &method_id,
                                       const std::vector<jni_local_value_ref> &parameters);
        
        static jni_local_ref<jobject> new_object(JNIEnv *jni, const jni_local_ref<jclass> &klass, const jmethodID &method_id,
                                                 const std::vector<jni_local_value_ref> &parameters);

        static void register_natives(JNIEnv *jni, const std::string &klass_name, const jni_global_ref<jclass> &klass,
                                     const std::vector<jni_native_method> &
                                     methods);
        static jni_local_ref<jobject> get_object_array_element(JNIEnv *jni, const std::pair<jni_local_ref<jobjectArray>, size_t> &array, int pos);

        static void unregister_natives(JNIEnv *jni, const std::string &klass_name, const jni_global_ref<jclass> &klass);

        static std::string get_string(JNIEnv *jni, const jni_local_ref<jstring> &string);
    };
}
