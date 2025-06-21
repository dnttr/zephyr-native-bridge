//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/jvmti/jvmti_object.hpp"

#include <algorithm>
#include <unordered_map>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/signatures/method_signature.hpp"

void znb_kit::jvmti_object::report_lacking_methods(std::unordered_multimap<std::string, jni_bridge_reference> map,
    std::vector<jni_native_method> &filtered)
{
    for (const auto& name : map | std::views::keys)
    {
        auto it = std::ranges::find_if(filtered, [&name](const jni_native_method& method) {
            return name == method.name_buffer.data();
        });

        if (it == filtered.end())
        {
            debug_print_cerr("[JNI] Unable to find method: '" + name + "' in the filtered methods.");
            debug_print_cerr("[JNI] The method may not have been registered or mapped correctly.");
            debug_print_cerr("[JNI] Method expects the following parameters:");

            if (auto [begin, end] = map.equal_range(name); begin != end)
            {
                for (auto p = begin; p != end; ++p)
                {
                    for (const auto& parameters = p->second.parameters; const auto& parameter : parameters)
                    {
                        debug_print_cerr("[JNI] Parameter: " + parameter);
                    }
                }
            }
        }
    }
}