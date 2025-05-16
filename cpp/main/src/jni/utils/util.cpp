//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/jni/utils/util.hpp"

#include <algorithm>
#include <unordered_set>

#include "ZNBKit/debug.hpp"

bool look_for_exceptions(JNIEnv *env)
{
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();

        return true;
    }

    return false;
}

jmethodID util::get_method_id(JNIEnv *env, const jclass &klass, const std::string &method_name,
                              const std::string &signature, const bool is_static)
{
    if (is_static)
    {
        return env->GetStaticMethodID(klass, method_name.c_str(), signature.c_str());
    }

    return env->GetMethodID(klass, method_name.c_str(), signature.c_str());
}

jmethodID util::get_method_id(JNIEnv *env, const std::string &klass_name, const std::string &method_name,
    const std::string &signature, const bool is_static)
{
    const auto klass = get_klass(env, klass_name);

    if (is_static)
    {
        return env->GetStaticMethodID(klass, method_name.c_str(), signature.c_str());
    }

    return env->GetMethodID(klass, method_name.c_str(), signature.c_str());
}

std::vector<jobject> util::get_methods(JNIEnv *env, const jclass &clazz)
{
    const auto method_id = util::get_method_id(env, "java/lang/Class", "getDeclaredMethods", "()[Ljava/lang/reflect/Method;", false);

    if (method_id == nullptr)
    {
        return {};
    }

    const auto array = reinterpret_cast<jobjectArray>(env->CallObjectMethod(clazz, method_id));

    if (look_for_exceptions(env))
    {
        throw std::runtime_error("Unable to get methods from class");
    }

    const auto array_size = env->GetArrayLength(array);

    std::vector<jobject> methods(array_size);

    for (int i = 0; i < array_size; ++i)
    {
        methods[i] = env->GetObjectArrayElement(array, i);

        if (look_for_exceptions(env))
        {
            throw std::runtime_error("Unable to get method from array");
        }
    }

    env->DeleteLocalRef(array);

    return methods;
}

jclass util::get_klass(JNIEnv *env, const std::string &name)
{
    const char *c_str = name.c_str();
    const auto klass = env->FindClass(c_str);

    if (look_for_exceptions(env))
    {
        throw std::runtime_error("Unable to find class '" + name + "'");
    }

    return klass;
}

std::string util::get_string(JNIEnv *env, const jstring &string, const bool release)
{
    const char *key = env->GetStringUTFChars(string, nullptr);

    if (look_for_exceptions(env))
    {
        throw std::runtime_error("Unable to get string from string");
    }

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

void util::delete_references(JNIEnv *env, const std::vector<jobject> &references)
{
    for (const auto &reference : references)
    {
        env->DeleteLocalRef(reference);
    }
}

std::vector<std::string> util::get_parameters(JNIEnv *env, const jobject &method)
{
    if (method == nullptr)
    {
        debug_print("get_parameters() is unable to obtain parameters as provided method object is null");
        return {};
    }

    const auto getParameterTypes_method_id  = get_method_id(env, "java/lang/reflect/Method", "getParameterTypes", "()[Ljava/lang/Class;", false);

    if (getParameterTypes_method_id == nullptr)
    {
        return {};
    }

    const auto getTypeName_method_id = get_method_id(env, "java/lang/Class", "getTypeName", "()Ljava/lang/String;", false);

    if (getTypeName_method_id == nullptr)
    {
        return {};
    }

    const auto array = reinterpret_cast<jobjectArray>(env->CallObjectMethod(method, getParameterTypes_method_id));

    if (look_for_exceptions(env))
    {
        throw std::runtime_error("Unable to get parameter types");
    }

    const auto array_size = env->GetArrayLength(array);

    if (array == nullptr)
    {
        debug_print("get_parameters() is unable to get array.");

        return {};
    }

    std::vector<std::string> methods(array_size);

    for (int i = 0; i < array_size; ++i)
    {
        const auto element = env->GetObjectArrayElement(array, i);

        if (look_for_exceptions(env))
        {
            throw std::runtime_error("Unable to get element from array");
        }

        const auto jstr = reinterpret_cast<jstring>(env->CallObjectMethod(element, getTypeName_method_id));

        if (look_for_exceptions(env))
        {
            throw std::runtime_error("Unable to get element from array");
        }

        const auto key = get_string(env, jstr);
        methods[i] = key;

        env->DeleteLocalRef(jstr);
        env->DeleteLocalRef(element);
    }

    env->DeleteLocalRef(array);

    return methods;
}

bool util::compare_parameters(const std::vector<std::string> &v1, const std::vector<std::string> &v2)
{
    if (v1.size() != v2.size()) {
        return false;
    }

    std::unordered_set set(v1.begin(), v1.end());

    if (set.size() != v1.size()) {
        return false;
    }

    return std::ranges::any_of(v2, [&set](const std::string &s) {
        return set.contains(s);
    });
}
