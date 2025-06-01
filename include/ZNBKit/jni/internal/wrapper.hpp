//
// Created by Damian Netter on 01/06/2025.
//

#pragma once

#include <jni.h>
#include <mutex>
#include <unordered_set>

#include "ZNBKit/debug.hpp"

namespace znb_kit
{
    inline thread_local std::unordered_set<jobject> local_refs;

    class global_tracker
    {
        static std::mutex mutex;
        static std::unordered_set<jobject> global_refs;

    public:
        static void add(const jobject ref)
        {
            std::lock_guard lock(mutex);

            if (ref != nullptr)
            {
                global_refs.insert(ref);
            }
        }

        static void remove(const jobject ref)
        {
            std::lock_guard lock(mutex);
            global_refs.erase(ref);
        }

        static size_t count()
        {
            std::lock_guard lock(mutex);
            return global_refs.size();
        }
    };

    std::mutex global_tracker::mutex;
    std::unordered_set<jobject> global_tracker::global_refs;

    class wrapper
    {
    public:
        static jobject add_local_ref(JNIEnv *env, const jobject obj)
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

        static void remove_local_ref(JNIEnv *env, const jobject obj)
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

        static jobject add_global_ref(JNIEnv *env, const jobject obj)
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

        static void remove_global_ref(JNIEnv *env, const jobject obj)
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

        static jclass search_for_class(JNIEnv *env, const std::string &name)
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

            if (klass == nullptr)
            {
                throw std::runtime_error("Cannot find class '" + name + "'");
            }

            local_refs.insert(klass);

            return klass;
        }

        static jmethodID get_method(JNIEnv *env, const jclass &klass, const std::string &method_name, const std::string &signature, const bool is_static)
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
            } else
            {
                method = env->GetMethodID(klass, method_name.c_str(), signature.c_str());
            }

            if (method == nullptr)
            {
                throw std::runtime_error("Method not found: " + method_name + " with signature: " + signature + " and static val: " + std::to_string(is_static));
            }

            return method;
        }
    };
}
