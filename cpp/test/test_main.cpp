//
// Created by Damian Netter on 10/05/2025.
//

#include <iostream>

#include "setup.hpp"

TEST_CASE("Klass")
{
    auto klass = vm->get_env()->FindClass("Native");
    REQUIRE(klass != nullptr);
}