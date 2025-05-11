//
// Created by Damian Netter on 11/05/2025.
//

#pragma once

#include <filesystem>
#include <jni.h>
#include <jvmti.h>
#include <optional>
#include <string>

#include "vm_object.hpp"

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

    static std::unique_ptr<vm_object> create_and_wrap_vm(const std::string &classpath)
    {
        vm_data vm_data;
        vm_data.version = JNI_VERSION_21;
        vm_data.classpath = classpath;

        jvmti_data jvmti_data;
        jvmti_data.version = JVMTI_VERSION_21;

        jvmti_data.capabilities.can_get_bytecodes = true;

        return create_and_wrap_vm(vm_data, jvmti_data);
    }

    static std::unique_ptr<vm_object> create_and_wrap_vm(const vm_data &vm_data, const std::optional<jvmti_data> jvmti_data)
    {
        std::cout << "Beginning initialization of vm." << std::endl;
        const auto [jvm, jni] = create_vm(vm_data);

        jvmtiEnv *jvmti = nullptr;
        std::cout << "------------------------------" << std::endl;

        if (jvmti_data.has_value())
        {
            std::cout << "JVMTI initialization requested." << std::endl;
            const auto data = jvmti_data.value();

            jvmti = get_jvmti(jvm, data.version);

            const auto capabilities = get_capabilities(jvmti, data);

            std::cout << "JVMTI capabilities requested." << std::endl;

            if (jvmti->AddCapabilities(&capabilities) != JVMTI_ERROR_NONE)
            {
                throw std::runtime_error("Failed to add jvmti capabilities.");
            }

            std::cout << "JVMTI capabilities added." << std::endl;
            std::cout << "JVMTI initialization completed." << std::endl;
            std::cout << "------------------------------" << std::endl;
        }

        std::cout << "Finished initialization of vm." << std::endl;

        return std::make_unique<vm_object>(vm_data.version, jvm, jvmti, jni);
    }
private:
    static jvmtiCapabilities get_capabilities(const jvmtiEnv *jvmti, const jvmti_data data)
    {
        if (jvmti == nullptr)
        {
            throw std::invalid_argument("JVMTI is null.");
        }

        jvmtiCapabilities capabilities = {};
        capabilities.can_get_bytecodes = data.capabilities.can_get_bytecodes;
        capabilities.can_generate_all_class_hook_events = data.capabilities.can_get_bytecodes;

        return capabilities;
    }

    static std::pair<JavaVM *, JNIEnv *> create_vm(const vm_data &vm_data)
    {
        const bool use_classpath = vm_data.classpath.has_value();
        const int options_count = use_classpath ? 1 : 0;

        JavaVM *jvm;
        JavaVMInitArgs vm_args;
        JavaVMOption options[options_count];

        std::string classpath_option;

        if (use_classpath)
        {
            std::cout << "------------------------------" << std::endl;
            std::cout << "Requesting file class loading." << std::endl;

            const auto classpath = vm_data.classpath.value();

            if (classpath.empty() || !std::filesystem::exists(classpath))
            {
                throw std::invalid_argument("Unable to determine classpath.");
            }

            classpath_option = "-Djava.class.path=" + classpath;

            options[0].optionString = const_cast<char *>(classpath_option.c_str());

            std::cout << "Loaded classes from: " << classpath << "." << std::endl;
        }

        vm_args.version = vm_data.version;
        vm_args.nOptions = options_count;
        vm_args.options = options;

        vm_args.ignoreUnrecognized = JNI_FALSE;

        JNIEnv *jni;

        if (JNI_CreateJavaVM(&jvm, reinterpret_cast<void **>(&jni), &vm_args) != JNI_OK)
        {
            throw std::runtime_error("Failed to initialize vm.");
        }

        return std::make_pair(jvm, jni);
    }

    static jvmtiEnv *get_jvmti(JavaVM *vm, const int version)
    {
        jvmtiEnv *jvmti = nullptr;
        if (vm->GetEnv(reinterpret_cast<void **>(&jvmti), version) != JNI_OK)
        {
            throw std::runtime_error("Failed to get jvmti.");
        }

        return jvmti;
    }
};
