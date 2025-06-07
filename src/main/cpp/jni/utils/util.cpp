//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/jni/utils/util.hpp"

#include <algorithm>
#include <unordered_set>

#include "ZNBKit/jni/internal/wrapper.hpp"

namespace znb_kit
{
    std::vector<jobject> get_methods(JNIEnv *env, const global_reference<jobject> &instance)
    {
        const auto method_id = wrapper::get_method(env, "java/lang/Class", "getDeclaredMethods", "()[Ljava/lang/reflect/Method;", false);

        if (method_id == nullptr)
        {
            return {};
        }

        const auto array_obj = wrapper::invoke_object_array_method(env, nullptr, instance, method_id, {});
        const auto array = array_obj.first.get();
        const auto array_size = array_obj.second;

        std::vector<jobject> methods(array_size);

        for (int i = 0; i < array_size; ++i)
        {
            methods[i] = env->GetObjectArrayElement(array, i);

            EXCEPT_CHECK(env);
        }

        return methods;
    }

    std::vector<std::string> get_parameters(JNIEnv *env, const global_reference<jobject> &instance)
    {
        const auto getParameterTypes_method_id  = wrapper::get_method(env, "java/lang/reflect/Method", "getParameterTypes", "()[Ljava/lang/Class;", false);
        const auto getTypeName_method_id = wrapper::get_method(env, "java/lang/Class", "getTypeName", "()Ljava/lang/String;", false);

        const auto array = wrapper::invoke_object_array_method(env, nullptr, instance, getParameterTypes_method_id, {});
        const auto array_size = array.second;

        std::vector<std::string> methods(array_size);

        for (int i = 0; i < array_size; ++i)
        {
            const auto element = wrapper::get_object_array_element(env, array, i);

            EXCEPT_CHECK(env);

            const auto ref = wrapper::invoke_string_method(env, nullptr, element, getTypeName_method_id, {});

            EXCEPT_CHECK(env);

            methods[i] = ref.get();
        }

        return methods;
    }

    bool compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2)
    {
        if (v1.size() != v2.size()) {
            return false;
        }

        std::unordered_set set(v1.begin(), v1.end());

        if (set.size() != v1.size()) {
            return false;
        }

        return std::ranges::all_of(v2, [&set](const std::string &s) {
            return set.contains(s);
        });
    }
}