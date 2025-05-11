//
// Created by Damian Netter on 11/05/2025.
//

#include "setup.hpp"
#include "jni/definitions/method/impl/void_method.hpp"

struct MethodProxyFixture {
    std::unique_ptr<klass_signature> klass;
    std::string name{"native_method1"};
    std::string signature{"()V"};

    MethodProxyFixture()
      : klass{ std::make_unique<klass_signature>(get_vm()->get_env(), "Native") }
    {}
};

TEST_CASE_METHOD(MethodProxyFixture, "Method proxy initialization", "[method_proxy]") {
    SECTION("Klass proxy initialization") {
        REQUIRE(klass);
    }

    SECTION("Method proxy initialization") {
        const void_method m{ get_vm()->get_env(),
                        klass.get(),
                        name,
                        signature,
                        std::nullopt,
                        false };
        REQUIRE(m.get_owner());
    }
}