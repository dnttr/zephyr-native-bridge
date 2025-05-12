//
// Created by Damian Netter on 11/05/2025.
//

#include "setup.hpp"
#include "jni/instance.hpp"
#include "jni/signatures/method/string_method.hpp"
#include "jni/signatures/method/void_method.hpp"

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

struct method_proxy_fixture {
    std::unique_ptr<klass_signature> klass;

    method_proxy_fixture()
      : klass{ std::make_unique<klass_signature>(get_vm()->get_env(), "Native") }
    {}
};

TEST_CASE_METHOD(method_proxy_fixture, "Method proxy initialization", "[method_proxy]") {
    SECTION("Klass proxy initialization") {
        REQUIRE(klass);
    }

    SECTION("Method proxy initialization") {
        std::string native_void_name{"native_method1"};
        std::string native_void_signature{"()V"};

        const void_method m_v{ get_vm()->get_env(),
                        klass.get(),
                        native_void_name,
                        native_void_signature,
                        std::nullopt,
                        false };
        REQUIRE(m_v.get_owner());

        std::string string_name{"y"};
        std::string string_signature{"()Ljava/lang/String;"};

        string_method m_s{ get_vm()->get_env(),
                        klass.get(),
                        string_name,
                        string_signature,
                        std::nullopt,
                        false };

        REQUIRE(m_s.get_owner());

        std::string instance_name = "<init>";
        std::string instance_signature = "()V";

        auto method = void_method(vm->get_env(), klass.get(), instance_name, instance_signature, std::nullopt, false);

        auto klass_instance = instance(get_vm(), method, {});
        REQUIRE(klass_instance.get_owner());

        std::vector<jvalue> empty_args;
        auto invoke_and_transform = m_s.invoke_and_transform(vm->get_env(), klass_instance.get_owner(), empty_args);
    }
}
