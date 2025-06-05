//
// Created by Damian Netter on 01/06/2025.
//

#pragma once

#include <jni.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "reference.hpp"

template <typename T>
using local_reference = znb_kit::reference<T, internal::policy::deleter<T, internal::policy::local_policy>>;

template <typename T>
using global_reference = znb_kit::reference<T, internal::policy::deleter<T, internal::policy::global_policy>>;

using local_value_reference = znb_kit::reference<jvalue, internal::policy::deleter_val<internal::policy::local_policy>>;
using global_value_reference = znb_kit::reference<jvalue, internal::policy::deleter_val<internal::policy::global_policy>>;

using string_reference = znb_kit::reference<jstring, internal::policy::deleter_string>;

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
        static local_reference<T> make_local(JNIEnv *env, T ref)
        {
            return local_reference<T>(ref, {env});
        }

        template <typename T>
        static global_reference<T> make_global(JNIEnv *env, T ref)
        {
            return global_reference<T>(ref, {env});
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

        static local_reference<jclass> search_for_class(JNIEnv *jni, const std::string &name,
                                                        const std::string &caller_file = __builtin_FILE(),
                                                        int caller_line = __builtin_LINE(),
                                                        const std::string &caller_function = __builtin_FUNCTION());

        template <typename KlassRefType>
        static jmethodID get_method(JNIEnv *jni, const KlassRefType &klass, const std::string &method_name,
                                    const std::string &signature, bool is_static);

        static jmethodID get_method(JNIEnv *jni, const std::string &name, const std::string &method,
                                    const std::string &signature, bool is_static);

        template <typename KlassRefType, typename ObjRefType>
        static local_reference<jobject> invoke_object_method(JNIEnv *jni, const KlassRefType &klass,
                                                             const ObjRefType &instance,
                                                             const jmethodID &method_id,
                                                             const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static std::pair<local_reference<jobjectArray>, size_t> invoke_object_array_method(
            JNIEnv *jni, const KlassRefType &klass,
            const ObjRefType &instance,
            const jmethodID &method_id,
            const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static string_reference invoke_string_method(JNIEnv *jni, const KlassRefType &klass,
                                                             const ObjRefType &instance,
                                                             const jmethodID &method_id,
                                                             const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static jbyte invoke_byte_method(JNIEnv *jni, const KlassRefType &klass,
                                        const ObjRefType &instance,
                                        const jmethodID &method_id,
                                        const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static jint invoke_int_method(JNIEnv *jni, const KlassRefType &klass,
                                      const ObjRefType &instance,
                                      const jmethodID &method_id, const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static jlong invoke_long_method(JNIEnv *jni, const KlassRefType &klass,
                                        const ObjRefType &instance,
                                        const jmethodID &method_id,
                                        const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static jshort invoke_short_method(JNIEnv *jni, const KlassRefType &klass,
                                          const ObjRefType &instance,
                                          const jmethodID &method_id,
                                          const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static jfloat invoke_float_method(JNIEnv *jni, const KlassRefType &klass,
                                          const ObjRefType &instance,
                                          const jmethodID &method_id,
                                          const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static jdouble invoke_double_method(JNIEnv *jni, const KlassRefType &klass,
                                            const ObjRefType &instance, const jmethodID &method_id,
                                            const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType, typename ObjRefType>
        static void invoke_void_method(JNIEnv *jni, const KlassRefType &klass,
                                       const ObjRefType &instance, const jmethodID &method_id,
                                       const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType>
        static local_reference<jobject> new_object(JNIEnv *jni, const KlassRefType &klass, const jmethodID &method_id,
                                                   const std::vector<local_value_reference> &parameters);

        template <typename KlassRefType>
        static void register_natives(JNIEnv *jni, const std::string &klass_name, const KlassRefType &klass,
                                     const std::vector<jni_native_method> &
                                     methods);

        template <typename ArrayRefType>
        static local_reference<jobject> get_object_array_element(JNIEnv *jni,
                                                                 const std::pair<ArrayRefType, size_t> &array, int pos);

        template <typename KlassRefType>
        static void unregister_natives(JNIEnv *jni, const std::string &klass_name, const KlassRefType &klass);

        static std::string get_string(JNIEnv *jni, const string_reference &string);
    };
}
