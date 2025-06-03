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

template<typename>
struct is_unique_ptr : std::false_type {};

template<typename U, typename D>
struct is_unique_ptr<std::unique_ptr<U, D>> : std::true_type {};

template<typename T>
constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;

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

    template <typename T>
    constexpr types get_type() {
        if constexpr (std::is_same_v<T, jboolean>)
            return types::BOOLEAN;
        else if constexpr (std::is_same_v<T, jbyte>)
            return types::BYTE;
        else if constexpr (std::is_same_v<T, jchar>)
            return types::CHAR;
        else if constexpr (std::is_same_v<T, jshort>)
            return types::SHORT;
        else if constexpr (std::is_same_v<T, jint>)
            return types::INT;
        else if constexpr (std::is_same_v<T, jlong>)
            return types::LONG;
        else if constexpr (std::is_same_v<T, jfloat>)
            return types::FLOAT;
        else if constexpr (std::is_same_v<T, jdouble>)
            return types::DOUBLE;
        else
            return types::OBJECT;
    }

    enum class jni_policy
    {
        LOCAL,
        GLOBAL,
        UNKNOWN
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

    struct jni_local_policy
    {
        void operator()(JNIEnv *jni, const jobject &ref) const {
            if (ref)
            {
                tracker_manager::local_refs.erase(ref);
                tracker_manager::local_ref_sources.erase(ref);

                jni->DeleteLocalRef(ref);
            }
        }
    };

    struct jni_global_policy
    {
        void operator()(JNIEnv *jni, const jobject &ref) const {
            if (ref)
            {
                global_tracker::remove(ref);

                jni->DeleteGlobalRef(ref);
            }
        }
    };

    template <typename T, typename policy>
    struct jni_reference_deleter
    {
        JNIEnv *jni;

        void operator()(T ref)
        {
            if (ref)
            {
                policy{}(jni, ref);
            }
        }
    };

    template <typename T>
    class jni_value
    {
        const T value;

        jni_policy policy;
        types type;

        explicit jni_value(const jvalue value, const jni_policy policy) : value(value), policy(policy)
        {

            auto types = get_type<T>();
            type = types;

            if (types != types::OBJECT)
            {
                if (policy != jni_policy::UNKNOWN)
                {
                    throw std::invalid_argument("Invalid combination. Type is not an object");
                }
            }
        }
    };

    struct jni_string_deleter
    {
        JNIEnv* env;
        jstring str;

        void operator()(const char* ptr) const {
            if (ptr) {
                env->ReleaseStringUTFChars(str, ptr);
            }
        }
    };
}
