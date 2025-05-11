#include <iostream>

#include "cpp/test/setup.hpp"

#include "vm_management.hpp"

int main(const int argc, char* argv[]) {
    Catch::Session session;

    std::cout << "--------" << "running tests" << "--------" << std::endl;
    std::cout << "Executing vm_dynamic_util -> initialize_vm" << std::endl;

    vm = vm_management::create_and_wrap_vm(ZNI_DST);

    return session.run(argc, argv);
}
