#include "ZNBKit/setup.hpp"

#include <iostream>

#include "ZNBKit/debug.hpp"
#include "ZNBKit/vm/vm_management.hpp"

using namespace znb_kit;

int main(const int argc, char *argv[])
{
    Catch::Session session;

    debug_print_ignore_formatting("--------> RUNNING TESTS <--------");

    std::vector<std::string> args;
    args.emplace_back(ZNI_JAR_PATH);

    vm = vm_management::create_and_wrap_vm(args);

    const int out = session.run(argc, argv);

    vm_management::cleanup_vm(vm->get_owner());

    return out;
}
