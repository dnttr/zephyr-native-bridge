//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <filesystem>
#include <jni.h>
#include <jvmti.h>
#include <optional>
#include <string>

#include "ZNBKit/vm_object.hpp"

class vm_management
{
public:
    struct jvmti_data
    {
        struct jvmti_capabilities
        {
            bool can_get_bytecodes = false;
            bool can_hook = false;
        };

        int version = JVMTI_VERSION_1_2;

        jvmti_capabilities capabilities = {};
    };

    struct vm_data
    {
        int version = JNI_VERSION_1_2;

        std::optional<std::string> classpath;
    };

    static std::unique_ptr<vm_object> create_and_wrap_vm(const std::string &classpath);

    static std::unique_ptr<vm_object> create_and_wrap_vm(const vm_data &vm_data, std::optional<jvmti_data> jvmti_data);

private:
    static jvmtiCapabilities get_capabilities(const jvmtiEnv *jvmti, jvmti_data data);

    static std::pair<JavaVM *, JNIEnv *> create_vm(const vm_data &vm_data);

    static jvmtiEnv *get_jvmti(JavaVM *vm, int version);
};
