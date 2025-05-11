//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#define ZNI_DST "@ZNI_DST@"

#include <catch2/catch_all.hpp>

#include "vm_object.hpp"

inline std::unique_ptr<vm_object> vm = nullptr;

inline vm_object *get_vm()
{
    if (vm == nullptr)
    {
        throw std::runtime_error("vm not initialized");
    }

    return vm.get();
}
