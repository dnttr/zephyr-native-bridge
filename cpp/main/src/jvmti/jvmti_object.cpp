//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/jvmti/jvmti_object.hpp"

#include <algorithm>
#include <map>
#include <unordered_map>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"
#include "ZNBKit/jni/signatures/method/string_method.hpp"

namespace znb_kit {
    template std::pair<std::vector<JNINativeMethod>, size_t>
    jvmti_object::try_mapping_methods<
        void, jobject, jstring, jint, jlong,
        jboolean, jfloat, jdouble, jbyte, jchar, jshort
    >(JNIEnv*, const klass_signature&,
       const std::unordered_multimap<std::string, Reference>&);

    // create_mappings
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<void>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jobject>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jstring>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jint>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jlong>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jboolean>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jfloat>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jdouble>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jbyte>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jchar>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);
    template std::pair<std::vector<JNINativeMethod>, size_t> jvmti_object::create_mappings<jshort>(JNIEnv*, const klass_signature&, const std::unordered_multimap<std::string, Reference>&);

}

void report_lacking_methods(std::multimap<std::string, znb_kit::Reference> map,
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
std::vector<JNINativeMethod> create_mappings(const std::unordered_multimap<std::string, znb_kit::Reference> &map,
    const std::vector<znb_kit::method_signature<T>> &methods)
{
    auto filtered = methods | std::ranges::views::transform([&map](const znb_kit::method_signature<T>& method) -> std::optional<JNINativeMethod> {
        auto [begin, end] = map.equal_range(method.name);
        auto it = std::ranges::find_if(begin, end, [&method](const auto& pair) {
            return compare_parameters(pair.second.parameters, method.parameters);
        });

        if (it == end)
            return std::nullopt;

        return JNINativeMethod{
            strdup(method.name.c_str()),
            strdup(method.signature.c_str()),
            it->second.has_func() ? it->func_ptr : nullptr
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
std::pair<std::vector<JNINativeMethod>, size_t> znb_kit::jvmti_object::create_mappings(JNIEnv *env,
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
std::pair<std::vector<JNINativeMethod>, size_t> znb_kit::jvmti_object::try_mapping_methods(JNIEnv *env,
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

void znb_kit::jvmti_object::clear_mapped_methods(const std::vector<JNINativeMethod> &vector)
{
    for (const auto native_method : vector)
    {
        free(native_method.name);
        free(native_method.signature);
    }
}