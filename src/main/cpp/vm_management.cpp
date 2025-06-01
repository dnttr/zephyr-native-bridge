//
// Created by Damian Netter on 12/05/2025.
//

#include "ZNBKit/vm_management.hpp"

#include "ZNBKit/debug.hpp"

std::unique_ptr<znb_kit::vm_object> znb_kit::vm_management::create_and_wrap_vm(const std::string &classpath)
{
    vm_data vm_data;
    vm_data.version = JNI_VERSION_21;
    vm_data.classpath = classpath;

    jvmti_data jvmti_data;
    jvmti_data.version = JVMTI_VERSION_21;

    jvmti_data.capabilities.can_get_bytecodes = true;

    return create_and_wrap_vm(vm_data, jvmti_data);
}

std::unique_ptr<znb_kit::vm_object> znb_kit::vm_management::create_and_wrap_vm(const vm_data &vm_data,
                                                             const std::optional<jvmti_data> jvmti_data)
{
    debug_print("Beginning initialization of vm.");
    const auto [jvm, jni] = create_vm(vm_data);

    jvmtiEnv *jvmti = nullptr;
    debug_print("------------------------------");

    if (jvmti_data.has_value())
    {
        jvmti = get_jvmti(jvm, jvmti_data.value());
    }
    else
    {
        debug_print("JVMTI initialization skipped.");
    }

    debug_print("Finished initialization of vm.");

    return std::make_unique<vm_object>(vm_data.version, jvm, jvmti, jni);
}

std::unique_ptr<znb_kit::vm_object> znb_kit::vm_management::wrap_vm(JavaVM *jvm, const std::optional<jvmti_data> jvmti_data)
{
    if (jvm == nullptr)
    {
        throw std::invalid_argument("JavaVM is null.");
    }

    JNIEnv *jni = nullptr;
    if (jvm->GetEnv(reinterpret_cast<void **>(&jni), JNI_VERSION_21) != JNI_OK)
    {
        throw std::runtime_error("Failed to get JNIEnv from JavaVM.");
    }

    jvmtiEnv *jvmti = nullptr;
    if (jvmti_data.has_value())
    {
        jvmti = get_jvmti(jvm, jvmti_data.value());
    }

    return std::make_unique<vm_object>(JNI_VERSION_21, jvm, jvmti, jni);
}

void znb_kit::vm_management::cleanup_vm(JNIEnv *env, JavaVM *vm)
{
    checkRemainingRefs();

    if (vm) {
        vm->DestroyJavaVM();
    }
}

jvmtiCapabilities znb_kit::vm_management::get_capabilities(const jvmtiEnv *jvmti, const jvmti_data data)
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

std::pair<JavaVM *, JNIEnv *> znb_kit::vm_management::create_vm(const vm_data &vm_data)
{
    const bool use_classpath = vm_data.classpath.has_value();
    const int options_count = use_classpath ? 1 : 0;

    JavaVM *jvm;
    JavaVMInitArgs vm_args;
    JavaVMOption options[options_count];

    std::string classpath_option;

    if (use_classpath)
    {
        debug_print("------------------------------");
        debug_print("Requesting file class loading.");

        const auto classpath = vm_data.classpath.value();

        if (classpath.empty() || !std::filesystem::exists(classpath))
        {
            throw std::invalid_argument("Unable to determine classpath. [" + classpath + "]");
        }

        classpath_option = "-Djava.class.path=" + classpath;

        options[0].optionString = const_cast<char *>(classpath_option.c_str());

        debug_print("Loaded classes from: " + classpath + ".");
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

jvmtiEnv * znb_kit::vm_management::get_jvmti(JavaVM *vm, const int version)
{
    jvmtiEnv *jvmti = nullptr;
    if (vm->GetEnv(reinterpret_cast<void **>(&jvmti), version) != JNI_OK)
    {
        throw std::runtime_error("Failed to get jvmti.");
    }

    return jvmti;
}

jvmtiEnv * znb_kit::vm_management::get_jvmti(JavaVM *vm, const jvmti_data data)
{
    jvmtiEnv *jvmti = nullptr;

    debug_print("JVMTI initialization requested.");

    jvmti = get_jvmti(vm, data.version);

    const auto capabilities = get_capabilities(jvmti, data);

    debug_print("JVMTI capabilities requested.");

    if (jvmti->AddCapabilities(&capabilities) != JVMTI_ERROR_NONE)
    {
        throw std::runtime_error("Failed to add jvmti capabilities.");
    }

    debug_print("JVMTI capabilities added.");
    debug_print("JVMTI initialization completed.");
    debug_print("------------------------------");

    return jvmti;
}
