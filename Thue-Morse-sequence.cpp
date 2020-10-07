#include <bit>
#include <ranges>
#include <iostream>

int main()
{
    for (const std::uint64_t n : std::views::iota(0))
        std::cout << std::popcount(n) % 2;
}
