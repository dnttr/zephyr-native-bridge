//
// Created by Damian Netter on 10/05/2025.
//

#include <iostream>
#include "../../include/ZNBKit/setup.hpp"

TEST_CASE("javavm internal methods availability")
{
    REQUIRE(vm->get_owner());
}

TEST_CASE("jni internal methods availability")
{
    REQUIRE(vm->get_env());
    REQUIRE(vm->get_env()->GetVersion() > 0);
}

TEST_CASE("jvmti internal methods availability")
{
    REQUIRE(vm->get_jvmti() != nullptr);

    int version;
    vm->get_jvmti()->get_owner()->GetVersionNumber(&version);
    REQUIRE(version > 0);
}