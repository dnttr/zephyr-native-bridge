#include "ZNBKit/setup.hpp"

#include <iostream>

#include "../../../include/ZNBKit/debug.hpp"
#include "ZNBKit/vm_management.hpp"

using namespace znb_kit;

int main(const int argc, char* argv[]) {
    Catch::Session session;

    debug_print("--------> Running tests <--------");

    vm = vm_management::create_and_wrap_vm(ZNI_DST);

    const int out = session.run(argc, argv);

    vm_management::cleanup_vm(vm->get_owner());

    return out;
}
