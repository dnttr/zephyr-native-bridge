#include "ZNBKit/setup.hpp"

#include <iostream>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/vm/vm_management.hpp"

using namespace znb_kit;

int main(const int argc, char* argv[]) {
    Catch::Session session;

    debug_print_ignore_formatting("--------> RUNNING TESTS <--------");

    vm = vm_management::create_and_wrap_vm(ZNI_JAR_PATH);

    const int out = session.run(argc, argv);

    vm_management::cleanup_vm(vm->get_owner());

    return out;
}
