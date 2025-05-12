//
// Created by Damian Netter on 11/05/2025.
//

#include "setup.hpp"
#include "jni/instance.hpp"
#include "jni/signatures/method/void_method.hpp"

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



    }
}
