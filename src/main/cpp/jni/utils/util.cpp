//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/jni/utils/util.hpp"

#include <algorithm>
#include <unordered_set>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/internal/wrapper.hpp"

namespace znb_kit
{
    std::vector<jobject> get_methods(JNIEnv *env, const jobject &instance)
    {
        const auto method_id = wrapper::get_method(env, "java/lang/Class", "getDeclaredMethods", "()[Ljava/lang/reflect/Method;", false);

        if (method_id == nullptr)
        {
            return {};
        }

        const auto array = reinterpret_cast<jobjectArray>(wrapper::invoke_object_method(env, nullptr, instance, method_id, {}));
        const auto array_size = env->GetArrayLength(array);

        std::vector<jobject> methods(array_size);

        for (int i = 0; i < array_size; ++i)
        {
            methods[i] = env->GetObjectArrayElement(array, i);

            EXCEPT_CHECK(env);
        }

        wrapper::remove_local_ref(env, array);

        return methods;
    }

    std::string get_string(JNIEnv *env, const jstring &string, const bool release)
    {
        const char *key = env->GetStringUTFChars(string, nullptr);

        EXCEPT_CHECK(env);

        if (key == nullptr)
        {
            debug_print("get_string() is unable to get UTF string");
            return {};
        }

        std::string str(key);

        if (release)
        {
            env->ReleaseStringUTFChars(string, key);
        }

        return str;
    }

    void delete_references(JNIEnv *env, const std::vector<jobject> &references)
    {
        for (const auto &reference : references)
        {
            wrapper::remove_local_ref(env, reference);
        }
    }

    std::vector<std::string> get_parameters(JNIEnv *env, const jobject &instance)
    {
        const auto getParameterTypes_method_id  = wrapper::get_method(env, "java/lang/reflect/Method", "getParameterTypes", "()[Ljava/lang/Class;", false);
        const auto getTypeName_method_id = wrapper::get_method(env, "java/lang/Class", "getTypeName", "()Ljava/lang/String;", false);

        const auto array = reinterpret_cast<jobjectArray>(wrapper::invoke_object_method(env, nullptr, instance, getParameterTypes_method_id, {}));
        const auto array_size = env->GetArrayLength(array);

        std::vector<std::string> methods(array_size);

        for (int i = 0; i < array_size; ++i)
        {

            const auto element = env->GetObjectArrayElement(array, i);

            EXCEPT_CHECK(env);

            const auto jstr = reinterpret_cast<jstring>(wrapper::invoke_object_method(env, nullptr, element, getTypeName_method_id, {}));

            EXCEPT_CHECK(env);

            const auto key = get_string(env, jstr);
            methods[i] = key;

            wrapper::remove_local_ref(env, jstr);
            wrapper::remove_local_ref(env, element);
        }

        wrapper::remove_local_ref(env, array);

        return methods;
    }

    bool compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2)
    {
        if (v1.size() != v2.size()) {
            debug_print("Size mismatch: " + std::to_string(v1.size()) + " vs " + std::to_string(v2.size()));
            return false;
        }

        for (size_t i = 0; i < v1.size(); ++i) {

            if (v1[i] != v2[i]) {
                debug_print("Mismatch at position " + std::to_string(i));
                return false;
            }
        }

        debug_print("[JNI] All parameters match");
        return true;
    }
}
