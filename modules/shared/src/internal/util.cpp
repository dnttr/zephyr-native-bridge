//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/internal/util.hpp"

#include <algorithm>
#include <unordered_set>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/internal/wrapper.hpp"

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

    bool compare_parameters(const std::string &method_name, const std::vector<std::string> &expected, const std::vector<std::string> &probable)
    {
        if (expected.size() != probable.size()) {
            debug_print("[JNI] For '" + method_name + "' occurred mismatch, with expected size of: " + std::to_string(expected.size()) + " and received " + std::to_string(probable.size()));

            for (const auto& parameter : probable)
            {
                debug_print("[JNI] Received parameter: " + parameter);
            }

            return false;
        }

        for (size_t i = 0; i < expected.size(); ++i) {
            if (expected[i] != probable[i]) {
                debug_print_cerr("[JNI] For '" + method_name + "' occurred a mismatch at position " + std::to_string(i) + " which evaluates to: '" + probable[i] + "' instead of expected '" + expected[i] + "'.");
                return false;
            }
        }

        debug_print("[JNI] For method '" + method_name + "' all parameters match, with compared amount of: " + std::to_string(expected.size()) + ".");
        return true;
    }
}
