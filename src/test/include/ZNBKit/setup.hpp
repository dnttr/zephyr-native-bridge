//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#include <catch2/catch_all.hpp>

#include "ZNBKit/vm_object.hpp"

#include "config.hpp"

inline std::unique_ptr<znb_kit::vm_object> vm = nullptr;

inline znb_kit::vm_object *get_vm()
{
    if (vm == nullptr)
    {
        throw std::runtime_error("vm not initialized");
    }

    return vm.get();
}
