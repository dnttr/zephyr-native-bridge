//
// Created by Damian Netter on 10/05/2025.
//

#pragma once

#include <filesystem>
#include <iostream>
#include <jni.h>
#include <jvmti.h>
#include <stdexcept>
#include <string>

#include "vm_object.hpp"

class vm_dynamic_util {

    std::unique_ptr<vm_object> vm;

    jvmtiEnv *initialize_jvmti(JavaVM *vm) const
    {
        jvmtiEnv *jvmti = nullptr;

        if (is_vm_initialized())
        {
            throw std::runtime_error("vm is already initialized");
        }

        if (vm->GetEnv(reinterpret_cast<void **>(&jvmti), JVMTI_VERSION_21) != JNI_OK)
        {
            throw std::runtime_error("failed to get jvmtiEnv");
        }

        jvmtiCapabilities capabilities = {};
        jvmti->GetPotentialCapabilities(&capabilities);
        capabilities.can_get_bytecodes = true;

        if (jvmti->AddCapabilities(&capabilities) != JVMTI_ERROR_NONE)
        {
            throw std::runtime_error("failed to add jvmti capabilities");
        }

        return jvmti;
    }
public:

    vm_object *initialize_vm(const std::string &target)
    {
        if (is_vm_initialized())
        {
            throw std::runtime_error("Unable to initialize vm twice");
        }

        if (target.empty() || !std::filesystem::exists(target))
        {
            throw std::invalid_argument("Unable to determine classpath");
        }

        std::cout << "Determined vm classpath: " << target << std::endl;

        const std::string classpath = "-Djava.class.path=" + target;

        JavaVM *vm = nullptr;
        JavaVMInitArgs vm_args;
        JavaVMOption options[1];
        JNIEnv *env;

        options[0].optionString = const_cast<char *>(classpath.c_str());

        vm_args.version = JNI_VERSION_21;

        vm_args.nOptions = 1;
        vm_args.options = options;

        vm_args.ignoreUnrecognized = JNI_FALSE;

        if (JNI_CreateJavaVM(&vm, reinterpret_cast<void **>(&env), &vm_args) != JNI_OK)
        {
            throw std::runtime_error("failed to initialize vm");
        }

        this->vm = std::make_unique<vm_object>(vm, initialize_jvmti(vm), env);

        return this->vm.get();
    }

    void destroy()
    {
        if (vm == nullptr)
        {
            return;
        }

        vm.reset();
    }

    [[nodiscard]] bool is_vm_initialized() const
    {
        return vm != nullptr;
    }
};
