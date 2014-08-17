#include <cstdlib>
#include <iostream>
#include <limits>

int main()
{
    // something is fucking wrong with language in here:
    std::cout << std::abs(1 - std::numeric_limits<int>::min())
      << '\n' << std::abs(1 - std::numeric_limits<long>::min())
      << '\n' << std::abs(0 - std::numeric_limits<int>::min())
      << '\n' << std::abs(0 - std::numeric_limits<long>::min())
      << '\n';
    // believe it or not, but imho std::abs should return value of UNSIGNED type
}


