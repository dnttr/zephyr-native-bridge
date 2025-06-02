//
// Created by Damian Netter on 11/05/2025.
//

#include <iostream>

#include "ZNBKit/setup.hpp"
#include "ZNBKit/jni/instance.hpp"
#include "ZNBKit/jni/signatures/method/string_method.hpp"
#include "ZNBKit/jni/signatures/method/void_method.hpp"

/*
 * This test is designed to verify the functionality of dynamic method mapping in JVMTI.
 * This functionality allows us to map Java methods to C++ functions at runtime, without prior knowledge of the method signatures.
 * I'll write it in docs later
 */

using namespace znb_kit;

struct method_proxy_fixture {
};

void ffi_example_method(JNIEnv *env) //need to look into whether jobject is needed here
{
    std::cout << "ffi_zm_open_session called" << std::endl;
}

TEST_CASE_METHOD(method_proxy_fixture, "JVMTI method mapping", "[jvmti]") {
    SECTION("Method proxy initialization") {
        const auto klass = klass_signature(get_vm()->get_env(), "org/dnttr/zephyr/bridge/Native");

        const auto jni_env = vm->get_env();
        const auto jvmti_env = vm->get_jvmti()->get().get_owner();

        if (jvmti_env == nullptr || jni_env == nullptr)
        {
            std::cout << "jvmti_env is null" << std::endl;
        }

        jvmti_object jvmti(jni_env, jvmti_env);
        const std::unordered_multimap<std::string, jni_bridge_reference> jvm_methods_map = {
            {"native_method1", jni_bridge_reference(&ffi_example_method, {})}};


        auto mapped = jvmti.try_mapping_methods<void>(klass, jvm_methods_map);
        //TODO: finish the test
        //move it somewhere else, it shouldn't be here
    }
}
