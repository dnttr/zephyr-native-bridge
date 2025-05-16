//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/jvmti/jvmti_object.hpp"

#include <algorithm>
#include <map>
#include <unordered_map>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/signatures/klass_signature.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"
#include "ZNBKit/jni/utils/util.hpp"

template <typename T>
method_signature<T> jvmti_object::get_method_signature(JNIEnv *env, const jobject &method)
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
std::vector<method_signature<T>> jvmti_object::look_for_method_signatures(JNIEnv *env,
    const jclass &klass)
{
    const auto objects = util::get_methods(env, klass);

    std::vector<method_signature<T>> descriptors;

    for (auto &object : objects)
    {
        auto method_descriptor = get_method_signature<T>(env, jvmti, object);
        descriptors.push_back(std::move(method_descriptor));
    }

    return descriptors;
}

template <typename T>
method_signature<T> jvmti_object::get_method_signature(JNIEnv *env,
    const jclass klass,
    std::string method_name,
    const std::vector<std::string> parameters)
{
    for (auto &object : util::get_methods(env, klass))
    {
        if (auto method_descriptor = get_method_signature<T>(env, jvmti, object); method_name.compare(method_descriptor.name))
        {
            if (util::compare_parameters(parameters, method_descriptor.parameters))
            {
                return method_descriptor;
            }
        }
    }

    return nullptr;
}

void report_lacking_methods(std::multimap<std::string, Reference> map,
    std::vector<JNINativeMethod> &filtered)
{
    for (const auto& name : map | std::views::keys)
    {
        auto it = std::ranges::find_if(filtered, [&name](const JNINativeMethod& method) {
            return name == method.name;
        });

        if (it == filtered.end())
        {
            debug_print("map_methods() was unable to find method: " + name);

            if (auto [begin, end] = map.equal_range(name); begin != end)
            {
                debug_print("map_methods() is expecting parameters:");
                for (auto p = begin; p != end; ++p)
                {
                    for (const auto& parameters = p->second.parameters; const auto& parameter : parameters)
                    {
                        debug_print("Parameter: " + parameter);
                    }
                }
            }
        }
    }
}

template <typename T>
std::vector<JNINativeMethod> create_mappings(const std::unordered_multimap<std::string, Reference> &map,
    const std::vector<method_signature<T>> &methods)
{
    auto filtered = methods | std::ranges::views::transform([&map](const method_signature<T>& method) -> std::optional<JNINativeMethod> {
        auto [begin, end] = map.equal_range(method.name);
        auto it = std::ranges::find_if(begin, end, [&method](const auto& pair) {
            return util::compare_parameters(pair.second.parameters, method.parameters);
        });

        if (it == end)
            return std::nullopt;

        return JNINativeMethod{
            strdup(method.name.c_str()),
            strdup(method.signature.c_str()),
            it->second.func_ptr
        };
    })
    | std::ranges::views::filter([](const auto& opt) {
        return opt.has_value();
    })
    | std::ranges::views::transform([](const auto& opt) {
        return *opt;
    });

    return std::ranges::to<std::vector<JNINativeMethod>>(filtered);
}

template <typename T>
std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings(JNIEnv *env,
    const klass_signature &klass,
    const std::unordered_multimap<std::string, Reference> &map)
    {

    const std::vector<method_signature<T>> methods = look_for_method_signatures<T>(env, klass);

    auto filtered_mappings = create_mappings(map, methods);

    size_t size = std::ranges::distance(filtered_mappings);

    if (size != map.size())
    {
        debug_print("map_methods() was unable to find all methods " + std::string(size + "/" +  map.size()));

        report_lacking_methods(map, filtered_mappings);

        return {};
    }

    return std::make_pair(filtered_mappings, size);
}

template <typename... Ts>
std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::try_mapping_methods(JNIEnv *env,
    const klass_signature &klass, const std::unordered_multimap<std::string, Reference> &map)
{
    std::vector<JNINativeMethod> mapped_methods;
    size_t total = 0;

    ([&] {
        auto [methods, count] = create_mappings<Ts>(env, klass, map);
        mapped_methods.insert(methods.end(), methods.begin(), methods.end());
        total += count;
    }(), ...);

    return {mapped_methods, total};
}

void jvmti_object::clear_mapped_methods(const std::vector<JNINativeMethod> &vector)
{
    for (const auto native_method : vector)
    {
        free(native_method.name);
        free(native_method.signature);
    }
}
