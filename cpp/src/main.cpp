#include <iostream>

extern "C" {
    int test(int a, int b);
}

int main() {
    const int result = test(5, 7);
    std::cout << "Result from Rust: " << result << std::endl;
    return 0;
}
