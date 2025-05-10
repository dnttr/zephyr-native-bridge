//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#include "jni/proxies/klass_proxy.hpp"
#include "jni/proxies/method_proxy.hpp"

class invoker {

    static bool is_definition_valid(const klass_proxy *klass_definition, const method_proxy *method_definition)
    {
        if (klass_definition == nullptr || method_definition == nullptr)
        {
            return false;
        }

        return true;
    }
};
