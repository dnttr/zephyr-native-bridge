//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/jvmti/jvmti_object.hpp"

#include <algorithm>
#include <unordered_map>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"

void znb_kit::jvmti_object::report_lacking_methods(std::unordered_multimap<std::string, reference> map,
    std::vector<native_method> &filtered)
{
    for (const auto& name : map | std::views::keys)
    {
        auto it = std::ranges::find_if(filtered, [&name](const znb_kit::native_method& method) {
            return name == method.name_buffer.data();
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

void znb_kit::jvmti_object::clear_mapped_methods(const std::vector<JNINativeMethod> &vector)
{
    for (const auto native_method : vector)
    {
        free(native_method.name);
        free(native_method.signature);
    }
}
