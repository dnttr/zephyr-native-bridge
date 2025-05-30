//
// Created by Damian Netter on 30/05/2025.
//

#include "ZNBKit/jvmti/factory.hpp"

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/signatures/method/byte_method.hpp"
#include "ZNBKit/jni/signatures/method/int_method.hpp"
#include "ZNBKit/jni/signatures/method/long_method.hpp"
#include "ZNBKit/jni/signatures/method/object_method.hpp"
#include "ZNBKit/jni/signatures/method/short_method.hpp"
#include "ZNBKit/jni/signatures/method/string_method.hpp"
#include "ZNBKit/jni/signatures/method/void_method.hpp"

#define ACC_STATIC 0x0008

namespace znb_kit
{
    template std::unique_ptr<method_signature<void>> factory::get_method_signature<void>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jobject>> factory::get_method_signature<jobject>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jstring>> factory::get_method_signature<jstring>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jint>> factory::get_method_signature<jint>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jlong>> factory::get_method_signature<jlong>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jboolean>> factory::get_method_signature<jboolean>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jfloat>> factory::get_method_signature<jfloat>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jdouble>> factory::get_method_signature<jdouble>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jbyte>> factory::get_method_signature<jbyte>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jchar>> factory::get_method_signature<jchar>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);
    template std::unique_ptr<method_signature<jshort>> factory::get_method_signature<jshort>(JNIEnv *, jvmtiEnv *, jclass, const jobject &);

    template std::vector<std::unique_ptr<method_signature<void>>> factory::look_for_method_signatures<void>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jobject>>> factory::look_for_method_signatures<jobject>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jstring>>> factory::look_for_method_signatures<jstring>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jint>>> factory::look_for_method_signatures<jint>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jlong>>> factory::look_for_method_signatures<jlong>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jboolean>>> factory::look_for_method_signatures<jboolean>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jfloat>>> factory::look_for_method_signatures<jfloat>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jdouble>>> factory::look_for_method_signatures<jdouble>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jbyte>>> factory::look_for_method_signatures<jbyte>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jchar>>> factory::look_for_method_signatures<jchar>(JNIEnv *, jvmtiEnv *, const jclass &);
    template std::vector<std::unique_ptr<method_signature<jshort>>> factory::look_for_method_signatures<jshort>(JNIEnv *, jvmtiEnv *, const jclass &);

    template std::unique_ptr<method_signature<void>> factory::get_method_signature<void>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jobject>> factory::get_method_signature<jobject>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jstring>> factory::get_method_signature<jstring>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jint>> factory::get_method_signature<jint>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jlong>> factory::get_method_signature<jlong>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jboolean>> factory::get_method_signature<jboolean>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jfloat>> factory::get_method_signature<jfloat>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jdouble>> factory::get_method_signature<jdouble>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jbyte>> factory::get_method_signature<jbyte>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jchar>> factory::get_method_signature<jchar>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jshort>> factory::get_method_signature<jshort>(JNIEnv *, jvmtiEnv *, jclass, std::string, std::vector<std::string>);
}

template <typename T>
std::unique_ptr<znb_kit::method_signature<T>> znb_kit::factory::get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, jclass klass, const jobject &method)
{
    const auto method_id = jni->FromReflectedMethod(method);

    char *name;
    char *signature;

    if (const jvmtiError error = jvmti->GetMethodName(method_id, &name, &signature, nullptr); error !=
        JVMTI_ERROR_NONE)
    {
        char *buffer;
        jvmti->GetErrorName(error, &buffer);
        debug_print("get_method_descriptor() has encountered an error: " + std::string(buffer));
        jvmti->Deallocate(reinterpret_cast<unsigned char *>(buffer));

        return nullptr;
    }

    auto name_duplicate = std::string(name);
    auto signature_duplicate = std::string(signature);

    jint modifiers;
    jvmti->GetMethodModifiers(method_id, &modifiers);
    bool is_static = (modifiers & ACC_STATIC) != 0;

    if (signature)
    {
        jvmti->Deallocate(reinterpret_cast<unsigned char *>(signature));
    }
    if (name)
    {
        jvmti->Deallocate(reinterpret_cast<unsigned char *>(name));
    }

    auto parameters = get_parameters(jni, method);

    const auto kls_sig = std::make_shared<klass_signature>(jni, klass);

    if constexpr (std::is_same_v<T, jobject>) {
        return std::make_unique<object_method>(jni, kls_sig.get(), name_duplicate, signature_duplicate, parameters, is_static);
    } else if constexpr (std::is_same_v<T, jshort>) {
        return std::make_unique<short_method>(jni, kls_sig.get(), name_duplicate, signature_duplicate, parameters, is_static);
    } else if constexpr (std::is_same_v<T, jbyte>) {
        return std::make_unique<byte_method>(jni, kls_sig.get(), name_duplicate, signature_duplicate, parameters, is_static);
    } else if constexpr (std::is_same_v<T, jint>) {
        return std::make_unique<int_method>(jni, kls_sig.get(), name_duplicate, signature_duplicate, parameters, is_static);
    } else if constexpr (std::is_same_v<T, jlong>) {
        return std::make_unique<long_method>(jni, kls_sig.get(), name_duplicate, signature_duplicate, parameters, is_static);
    } else if constexpr (std::is_same_v<T, jstring>) {
        return std::make_unique<string_method>(jni, kls_sig.get(), name_duplicate, signature_duplicate, parameters, is_static);
    } else if constexpr (std::is_same_v<T, void>) {
        return std::make_unique<void_method>(jni, kls_sig.get(), name_duplicate, signature_duplicate, parameters, is_static);
    }
    
    return nullptr;
}

template <typename T>
std::unique_ptr<znb_kit::method_signature<T>> znb_kit::factory::get_method_signature(JNIEnv *jni,
    jvmtiEnv *jvmti,
    const jclass klass,
    std::string method_name,
    const std::vector<std::string> parameters)
{
    for (auto &object : get_methods(jni, klass))
    {
        auto method_descriptor = get_method_signature<T>(jni, jvmti, klass, object);
        if (method_descriptor && method_name.compare(method_descriptor->name) == 0)
        {
            if (compare_parameters(parameters, method_descriptor->parameters))
            {
                return method_descriptor;
            }
        }
    }

    return nullptr;
}

template <typename T>
std::vector<std::unique_ptr<znb_kit::method_signature<T>>> znb_kit::factory::look_for_method_signatures(JNIEnv *jni, jvmtiEnv *jvmti, const jclass &klass)
{
    const auto objects = get_methods(jni, klass);
    std::vector<std::unique_ptr<method_signature<T>>> descriptors;

    for (auto &object : objects)
    {
        if (auto method_descriptor = get_method_signature<T>(jni, jvmti, klass, object)) {
            descriptors.push_back(std::move(method_descriptor));
        }
    }

    return descriptors;
}