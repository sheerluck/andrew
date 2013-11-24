#include <iostream>
#include <limits>
#include <iomanip>
using A = unsigned long long;
int main() {
    auto a    = A{0ULL};
    auto max  = std::numeric_limits<A>::max();
    auto step = int{1<<28};
    while (a++ < max) {
        if (0 == a % step) {
            std::cout << std::hex << std::showbase
              << std::setw (20) << a << std::endl;
        }
    }
}

