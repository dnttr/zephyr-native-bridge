//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#define ZNI_DST "@ZNI_DST@"

#include <catch2/catch_all.hpp>

#include "debug/vm_dynamic_util.hpp"
#include "debug/vm_object.hpp"

inline vm_object *vm = nullptr;

inline vm_object *get_vm()
{
    if (vm == nullptr)
    {
        throw std::runtime_error("vm not initialized");
    }

    return vm;
}
