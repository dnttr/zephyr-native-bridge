//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <jvmti.h>
#include <ranges>
#include <stdexcept>
#include <unordered_set>
#include <map>
#include <vector>

#include "jni/signatures/method/method_signature.hpp"
#include "jni/utils/util.hpp"

class jvmti_object
{
    jvmtiEnv *jvmti;

    template<typename ReturnType = jobject>
    method_signature<ReturnType> get_method_descriptor(JNIEnv *env, jvmtiEnv *jvmti, const jobject &method)
    {
        const auto method_id = env->FromReflectedMethod(method);

        char *name;
        char *signature;

        if (const jvmtiError error = jvmti->GetMethodName(method_id, &name, &signature, nullptr); error !=
            JVMTI_ERROR_NONE)
        {
            char *buffer;
            jvmti->GetErrorName(error, &buffer);
            std::cerr << "get_method_descriptor >>" << "Error occurred while invoking GetMethodName: " << buffer << std::endl;
            jvmti->Deallocate(reinterpret_cast<unsigned char *>(buffer));

            return method_signature<ReturnType>(nullptr);
        }

        auto name_duplicate = std::string(name);
        auto signature_duplicate = std::string(signature);

        if (signature)
        {
            jvmti->Deallocate(reinterpret_cast<unsigned char *>(signature));
        }
        if (name)
        {
            jvmti->Deallocate(reinterpret_cast<unsigned char *>(name));
        }

        auto parameters = Util::get_parameters(env, method);

        return method_signature<ReturnType>{name_duplicate, signature_duplicate, parameters};
    }

    template<typename ReturnType = jobject>
    std::vector<method_signature<ReturnType>> gather_method_descriptors(JNIEnv *env, jvmtiEnv *jvmti,
                                                  const std::vector<jobject> &objects)
    {
        std::vector<method_signature<ReturnType>> descriptors;

        for (auto &object : objects)
        {
            auto method_descriptor = get_method_descriptor<ReturnType>(env, jvmti, object);
            descriptors.push_back(std::move(method_descriptor));
        }

        return descriptors;
    }

    template<typename ReturnType = jobject>
    std::vector<JNINativeMethod> map_methods(
        const std::map<std::string, typename method_signature<ReturnType>::Reference> &map,
        const std::vector<method_signature<ReturnType>> &methods, 
        size_t *size)
    {
        if (size == nullptr)
        {
            std::cerr << "map_methods >> " << "Size cannot be nullptr" << std::endl;
            return {};
        }

        auto filtered = methods | std::ranges::views::filter([&map](const method_signature<ReturnType> &method)
            {
                return map.contains(method.name);
            }) | std::ranges::views::filter([&map](const method_signature<ReturnType> &method)
            {
                const auto expected_parameters = map.at(method.name).parameters;
                const auto declared_parameters = method.parameters;

                return compare_parameters(expected_parameters, declared_parameters);
            }) | std::ranges::views::transform([&map](const method_signature<ReturnType> &method)
            {
                char *name = strdup(method.name.c_str());
                char *signature = strdup(method.signature.c_str());

                if (name == nullptr || signature == nullptr)
                {
                    std::cerr << "map_methods >> " << "Failed to allocate memory for method name or signature" << std::endl;
                    return JNINativeMethod{nullptr, nullptr, nullptr};
                }

                return JNINativeMethod{name, signature, map.at(method.name).func_ptr
                };
            });

        *size = std::ranges::distance(filtered);

        if (*size != map.size())
        {
            std::cerr << "map_methods >> " << "Not all methods were found: " << *size << " / " << map.size() << std::endl;

            for (const auto &name : map | std::views::keys)
            {
                if (!std::ranges::any_of(filtered, [&name](const JNINativeMethod &method)
                    {
                        return name == method.name;
                    }))
                {
                    std::cerr << "map_methods >> " << "Method not found: " << name << std::endl;

                    if (auto parameters = map.at(name).parameters; !parameters.empty())
                    {
                        std::cerr << "map_methods >> " << "Expected parameters: ";

                        std::cerr << "[ ";

                        for (const auto& parameter : parameters)
                        {
                            std::cerr << parameter << " ";
                        }

                        std::cerr << "]" << std::endl;
                    }
                }
            }

            return {};
        }

        auto methods_ = std::ranges::to<std::vector<JNINativeMethod>>(filtered);

        return methods_;
    }

    /**
*    void clear_mapped_methods(const std::vector<JNINativeMethod> &vector)
{
    for (const auto native_method : vector)
    {
        free(native_method.name);
        free(native_method.signature);
    }

template<typename ReturnType = jobject>
void lookup_methods(std::string name, std::optional<std::vector<std::string>> parameters)
{
    // Implementation needed
}
}

     */
public:

    explicit jvmti_object(jvmtiEnv *jvmti): jvmti(jvmti)
    {
        if (jvmti == nullptr)
        {
            throw std::runtime_error("JVMTI is null");
        }
    }

    [[nodiscard]] jvmtiEnv *get_owner() const
    {
        return jvmti;
    }
};
