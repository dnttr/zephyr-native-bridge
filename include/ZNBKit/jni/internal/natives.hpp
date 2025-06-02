//
// Created by Damian Netter on 02/06/2025.
//

#pragma once

#include <vector>

#include "jni.h"
#include "ZNBKit/jni/signatures/klass_signature.hpp"

namespace znb_kit
{
/*    void register_methods(JNIEnv *jni, std::map<klass_signature, std::vector<method>> &methods_map)
    {
        for (const auto&[owner_ks, methods] : methods_map)
        {
            if (methods.empty())
            {
                continue;
            }

            wrapper::register_natives(jni, "", owner_ks.get_owner(), methods);
        }
    } */
}
