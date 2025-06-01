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
    std::unordered_map<jobject, ref_info> global_tracker::global_ref_sources;

    void global_tracker::add(const jobject &ref, const std::string &file, int line, const std::string &method)
    {
        std::lock_guard lock(mutex);

        if (ref != nullptr)
        {
            global_refs.insert(ref);
            global_ref_sources[ref] = {file, line, method};
        }
    }

    void global_tracker::remove(const jobject &ref)
    {
        std::lock_guard lock(mutex);
        global_refs.erase(ref);
        global_ref_sources.erase(ref);
    }

    size_t global_tracker::count()
    {
        std::lock_guard lock(mutex);
        return global_refs.size();
    }

    void global_tracker::dump_refs()
    {
        std::lock_guard lock(mutex);
        debug_print_cerr("Dumping " + std::to_string(global_refs.size()) + " global references:");

        for (const auto& ref : global_refs)
        {
            if (auto it = global_ref_sources.find(ref); it != global_ref_sources.end())
            {
                const auto& info = it->second;
                debug_print_cerr("Global ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                                " created at " + info.file + ":" + std::to_string(info.line) +
                                " in " + info.method);
            }
            else
            {
                debug_print_cerr("Global ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                                " (source unknown)");
            }
        }
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
                global_tracker::dump_refs();
            }

            if (!is_local_empty)
            {
                debug_print_cerr("Local references are not empty, potential memory leak detected.");
                dump_local_refs();
            }
        } else
        {
            debug_print_cerr("No references left. All good i think, unless not using wrapper, then well, you are on your own. :>");
        }
    }

    void wrapper::dump_local_refs()
    {
        debug_print_cerr("Dumping " + std::to_string(local_refs.size()) + " local references:");
        for (const auto& ref : local_refs)
        {
            auto it = local_ref_sources.find(ref);
            if (it != local_ref_sources.end())
            {
                const auto& info = it->second;
                debug_print_cerr("Local ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                               " created at " + info.file + ":" + std::to_string(info.line) +
                               " in " + info.method +
                               (info.details.empty() ? "" : " (" + info.details + ")"));
            }
            else
            {
                debug_print_cerr("Local ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                               " (source unknown)");
            }
        }
    }

    jobject wrapper::add_local_ref(JNIEnv *env, const jobject &obj,
                                  const std::string &file, int line,
                                  const std::string &method)
    {
        if (env == nullptr)
        {
            throw std::invalid_argument("Variable 'env' is null");
        }

        const auto ref = env->NewLocalRef(obj);

        if (ref)
        {
            local_refs.insert(ref);
            local_ref_sources[ref] = {file, line, method};
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
            local_ref_sources.erase(obj);
            env->DeleteLocalRef(obj);
        }
    }

    jobject wrapper::add_global_ref(JNIEnv *env, const jobject &obj,
                                   const std::string &file, int line,
                                   const std::string &method)
    {
        if (env == nullptr)
        {
            throw std::invalid_argument("Variable 'env' is null");
        }

        const auto ref = env->NewGlobalRef(obj);

        if (ref)
        {
            global_tracker::add(ref, file, line, method);
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

    jclass wrapper::search_for_class(JNIEnv *env, const std::string &name,
                               const std::string &caller_file,
                               const int caller_line,
                               const std::string &caller_function)
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

        std::string callsite = "Called from " + get_path(caller_file) + ":" +
                             std::to_string(caller_line) + " in " + caller_function;

        local_ref_sources[klass] = {
            __FILE__,
            __LINE__,
            __func__,
            "Class: " + name + " | " + callsite
        };

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
        const auto method_id = get_method(env, klass, method, signature, is_static);
        remove_local_ref(env, klass);

        return method_id;
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

        return add_local_ref(env, result, __FILE__, __LINE__, __func__);
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
