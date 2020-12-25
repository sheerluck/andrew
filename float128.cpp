// g++ -std=c++2a -lquadmath float128.cpp -o float128.exe
#include <iostream>
#include <memory>
#include <quadmath.h>
#include <ranges>

namespace views = std::ranges::views;

inline
std::ostream &
operator << (std::ostream &out, __float128 x) {
    auto format = "%.36Qg";
    auto n = quadmath_snprintf(NULL, 0, format, x);
    auto buf = std::make_unique<char []>(n+1);
    quadmath_snprintf(buf.get(), n+1, format, x);
    return
        out << buf.get();
}

int main()
{
    auto a = strtoflt128("1e4900", NULL);
    for (const auto& i : views::iota(1, 32'000))
    {
        auto b = a / 2.0;
        std::cout << i << ' ' << b << '\n';
        a = b;
    }
}
