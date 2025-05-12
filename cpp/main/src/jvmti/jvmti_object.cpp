//
// Created by Damian Netter on 12/05/2025.
//

#include "jvmti/jvmti_object.hpp"

#include <algorithm>

#include "debug.hpp"

template <typename T>
method_signature<T> jvmti_object::get_method_descriptor(JNIEnv *env, const jobject &method)
{
    const auto method_id = env->FromReflectedMethod(method);

    char *name;
    char *signature;

    if (const jvmtiError error = jvmti->GetMethodName(method_id, &name, &signature, nullptr); error !=
        JVMTI_ERROR_NONE)
    {
        char *buffer;
        jvmti->GetErrorName(error, &buffer);
        debug_print("get_method_descriptor() has encountered an error: " + std::string(buffer));
        jvmti->Deallocate(reinterpret_cast<unsigned char *>(buffer));

        return method_signature<T>(nullptr);
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

    auto parameters = util::get_parameters(env, method);

    return method_signature<T>{name_duplicate, signature_duplicate, parameters};
}

template <typename T>
std::vector<method_signature<T>> jvmti_object::gather_method_descriptors(JNIEnv *env,
    const jclass &klass)
{
    const auto objects = util::get_methods(env, klass);

    std::vector<method_signature<T>> descriptors;

    for (auto &object : objects)
    {
        auto method_descriptor = get_method_descriptor<T>(env, jvmti, object);
        descriptors.push_back(std::move(method_descriptor));
    }

    return descriptors;
}

template <typename T>
method_signature<T> find_method(JNIEnv *env, const jclass klass, std::string method_name, std::vector<std::string> parameters)
{
    auto objects = util::get_methods(env, klass);

    //TODO: implement find_method
    return nullptr;
}

template <typename T>
std::vector<JNINativeMethod> jvmti_object::map_methods(const std::map<std::string, Reference> &map,
    const std::vector<method_signature<T>> &methods, size_t *size)
{
    if (size == nullptr)
    {
        debug_print("map_methods() is unable to determine size");
        return {};
    }

    auto filtered = methods | std::ranges::views::filter([&map](const method_signature<T> &method)
           {
               return map.contains(method.name);
           }) | std::ranges::views::filter([&map](const method_signature<T> &method)
           {
               const auto expected_parameters = map.at(method.name).parameters;
               const auto declared_parameters = method.parameters;

               return compare_parameters(expected_parameters, declared_parameters);
           }) | std::ranges::views::transform([&map](const method_signature<T> &method)
           {
               char *name = strdup(method.name.c_str());
               char *signature = strdup(method.signature.c_str());

               if (name == nullptr || signature == nullptr)
               {
                   debug_print("map_methods() has failed to allocate memory for method name or signature");
                   return JNINativeMethod{nullptr, nullptr, nullptr};
               }

               return JNINativeMethod{name, signature, map.at(method.name).func_ptr
               };
           });

    *size = std::ranges::distance(filtered);

    if (*size != map.size())
    {
        debug_print("map_methods() was unable to find all methods " + std::string(*size + "/" +  map.size()));

        for (const auto &name : map | std::views::keys)
        {
            if (!std::ranges::any_of(filtered, [&name](const JNINativeMethod &method)
                {
                    return name == method.name;
                }))
            {
                debug_print("map_methods() was unable to find method: " + name);

                if (auto parameters = map.at(name).parameters; !parameters.empty())
                {
                    debug_print("map_methods() is expecting parameters:");


                    for (const auto& parameter : parameters)
                    {
                        debug_print("Parameter: " + parameter);
                    }
                }
            }
        }

        return {};
    }

    return std::ranges::to<std::vector<JNINativeMethod>>(filtered);
}

void jvmti_object::clear_mapped_methods(const std::vector<JNINativeMethod> &vector)
{
    for (const auto native_method : vector)
    {
        free(native_method.name);
        free(native_method.signature);
    }
}
