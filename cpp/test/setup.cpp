#include <iostream>

#include "cpp/test/setup.hpp"
#include "debug/vm_dynamic_util.hpp"

int main(const int argc, char* argv[]) {
    Catch::Session session;

    std::cout << "--------" << "running tests" << "--------" << std::endl;
    std::cout << "Executing vm_dynamic_util -> initialize_vm" << std::endl;

    vm_dynamic_util vm_util;

    vm = vm_util.initialize_vm(ZNI_DST);

    return session.run(argc, argv);
}