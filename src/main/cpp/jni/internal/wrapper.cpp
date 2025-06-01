//
// Created by Damian Netter on 01/06/2025.
//

#include "ZNBKit/jni/internal/wrapper.hpp"

#include <vector>

#include "ZNBKit/debug.hpp"

namespace znb_kit
{
    std::mutex global_tracker::mutex;
    std::unordered_set<jobject> global_tracker::global_refs;

    void global_tracker::add(const jobject &ref)
    {
        std::lock_guard lock(mutex);

        if (ref != nullptr)
        {
            global_refs.insert(ref);
        }
    }

    void global_tracker::remove(const jobject &ref)
    {
        std::lock_guard lock(mutex);
        global_refs.erase(ref);
    }

    size_t global_tracker::count()
    {
        std::lock_guard lock(mutex);
        return global_refs.size();
    }

    void wrapper::check_for_refs()
    {
        const bool is_global_empty = global_tracker::count() == 0;
        const bool is_local_empty = local_refs.empty();

        if (!is_global_empty || !is_local_empty)
        {
            debug_print_cerr("WARNING - Potential memory leak detected");

            debug_print_cerr("Global references count: " + std::to_string(global_tracker::count()));
            debug_print_cerr("Local references count: " + std::to_string(local_refs.size()));

            if (!is_global_empty)
            {
                debug_print_cerr("Global references are not empty, potential memory leak detected.");
            }

            if (!is_local_empty)
            {
                debug_print_cerr("Local references are not empty, potential memory leak detected.");
            }
        } else
        {
            debug_print_cerr("No references left. All good!");
        }
    }

    jobject wrapper::add_local_ref(JNIEnv *env, const jobject &obj)
    {
        if (env == nullptr)
        {
            throw std::invalid_argument("Variable 'env' is null");
        }

        const auto ref = env->NewLocalRef(obj);

        if (ref)
        {
            local_refs.insert(ref);
        }

        return ref;
    }

    void wrapper::remove_local_ref(JNIEnv *env, const jobject &obj)
    {
        if (env == nullptr)
        {
            throw std::invalid_argument("Variable 'env' is null");
        }

        if (obj)
        {
            local_refs.erase(obj);
            env->DeleteLocalRef(obj);
        }
    }

    jobject wrapper::add_global_ref(JNIEnv *env, const jobject &obj)
    {
        if (env == nullptr)
        {
            throw std::invalid_argument("Variable 'env' is null");
        }

        const auto ref = env->NewGlobalRef(obj);

        if (ref)
        {
            global_tracker::add(ref);
        }

        return ref;
    }

    void wrapper::remove_global_ref(JNIEnv *env, const jobject &obj)
    {
        if (env == nullptr)
        {
            throw std::invalid_argument("Variable 'env' is null");
        }

        if (obj)
        {
            global_tracker::remove(obj);
            env->DeleteGlobalRef(obj);
        }
    }

    jclass wrapper::search_for_class(JNIEnv *env, const std::string &name)
    {
        if (env == nullptr)
        {
            throw std::invalid_argument("Variable 'env' is null");
        }

        if (name.empty())
        {
            throw std::invalid_argument("Class name cannot be empty");
        }

        const auto klass = env->FindClass(name.c_str());

        EXCEPT_CHECK(env);

        if (klass == nullptr)
        {
            throw std::runtime_error("Cannot find class '" + name + "'");
        }

        local_refs.insert(klass);

        return klass;
    }

    jmethodID wrapper::get_method(JNIEnv *env, const jclass &klass, const std::string &method_name,
                                  const std::string &signature, const bool is_static)
    {
        if (env == nullptr || klass == nullptr)
        {
            throw std::invalid_argument("Variable 'env' or 'klass' is null");
        }

        if (method_name.empty() || signature.empty())
        {
            throw std::invalid_argument("Method name or signature cannot be empty");
        }

        jmethodID method = nullptr;

        if (is_static)
        {
            method = env->GetStaticMethodID(klass, method_name.c_str(), signature.c_str());
        }
        else
        {
            method = env->GetMethodID(klass, method_name.c_str(), signature.c_str());
        }

        EXCEPT_CHECK(env);

        if (method == nullptr)
        {
            throw std::runtime_error(
                "Method not found: " + method_name + " with signature: " + signature + " and static val: " +
                std::to_string(is_static));
        }

        return method;
    }

    jmethodID wrapper::get_method(JNIEnv *env, const std::string &name, const std::string &method,
                                  const std::string &signature, const bool is_static)
    {
        const auto klass = search_for_class(env, name);
        return get_method(env, klass, method, signature, is_static);
    }

    jobject wrapper::invoke_object_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);

        jobject result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = env->CallStaticObjectMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            result = env->CallObjectMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);

        return add_local_ref(env, result);
    }

    jbyte wrapper::invoke_byte_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                      const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);

        jbyte result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = env->CallStaticByteMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            result = env->CallByteMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);

        return result;
    }

    jint wrapper::invoke_int_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                    const jmethodID &method_id,
                                    const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);

        jint result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = env->CallStaticIntMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            result = env->CallIntMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);

        return result;
    }

    jlong wrapper::invoke_long_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                      const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);

        jlong result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = env->CallStaticLongMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            result = env->CallLongMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);

        return result;
    }

    jshort wrapper::invoke_short_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);

        jshort result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = env->CallStaticShortMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            result = env->CallShortMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);

        return result;
    }

    jfloat wrapper::invoke_float_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);

        jfloat result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = env->CallStaticFloatMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            result = env->CallFloatMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);

        return result;
    }

    jdouble wrapper::invoke_double_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);

        jdouble result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = env->CallStaticDoubleMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            result = env->CallDoubleMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);

        return result;
    }

    void wrapper::invoke_void_method(JNIEnv *env, const jclass &klass, const jobject &instance,
                                     const jmethodID &method_id,
                                     const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(env);
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            env->CallStaticVoidMethodA(klass, method_id, parameters.data());
        }
        else
        {
            if (instance == nullptr)
            {
                throw std::invalid_argument("Variable 'instance' is null");
            }

            env->CallVoidMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(env);
    }
}
