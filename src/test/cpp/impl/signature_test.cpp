//
// Created by Damian Netter on 11/05/2025.
//

#include <iostream>

#include "ZNBKit/setup.hpp"
#include "ZNBKit/jni/instance.hpp"
#include "ZNBKit/jni/signatures/method/string_method.hpp"
#include "ZNBKit/jni/signatures/method/void_method.hpp"

/*
 * This test is designed to test the method proxy and its ability to
 * invoke methods on a Java class. It does not test the actual Java
 * class itself, but rather the JNI interface and the method proxy.
 *
 * The Java class is assumed to be in the classpath and is loaded
 * using the JNI interface.
 *
 * It is scheduled for removal in the future.
 * It is a placeholder for the actual test implementation.
 */

using namespace znb_kit;

struct method_proxy_fixture {
};

void ffi_zm_open_session(JNIEnv *env, jobject)
{
    std::cout << "ffi_zm_open_session called" << std::endl;
}

TEST_CASE_METHOD(method_proxy_fixture, "Method proxy initialization", "[method_proxy]") {
    SECTION("Method proxy initialization") {
        const auto klass = klass_signature(get_vm()->get_env(), "org/dnttr/zephyr/bridge/Native");

        const auto jni_env = vm->get_env();
        const auto jvmti_env = vm->get_jvmti()->get().get_owner();

        if (jvmti_env == nullptr || jni_env == nullptr)
        {
            std::cout << "jvmti_env is null" << std::endl;
        }

        jvmti_object jvmti(jni_env, jvmti_env);
        const std::unordered_multimap<std::string, reference> jvm_methods_map = {
            {"native_method1", reference(&ffi_zm_open_session, {})}};


        auto mapped = jvmti.try_mapping_methods<void>(klass, jvm_methods_map);
        //TODO: finish the test
        //move it somewhere else, it shouldn't be here
    }
}
