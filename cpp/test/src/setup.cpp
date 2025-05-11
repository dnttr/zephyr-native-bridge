#include <iostream>

#include "cpp/test/include/setup.hpp"

#include "vm_management.hpp"

int main(const int argc, char* argv[]) {
    Catch::Session session;

    std::cout << "--------" << "> Running tests <" << "--------" << std::endl;

    vm = vm_management::create_and_wrap_vm(ZNI_DST);

    return session.run(argc, argv);
}
