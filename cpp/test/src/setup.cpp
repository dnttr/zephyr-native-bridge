#include "ZNBKit/setup.hpp"

#include <iostream>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/vm_management.hpp"

int main(const int argc, char* argv[]) {
    Catch::Session session;

    debug_print("--------> Running tests <--------");

    vm = vm_management::create_and_wrap_vm(ZNI_DST);

    return session.run(argc, argv);
}
