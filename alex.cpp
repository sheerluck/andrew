// выдать против числа - его частота в векторе
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>

constexpr double EPS = 0.2;
constexpr double EPS1 = 1/EPS;

template<typename A>
void
out(A a)
{
    for (const auto& elem : a) std::cout << elem << " ";
    std::cout << '\n';  
}

std::vector<int>
find_freq(const std::vector<double>& v)
{
    auto result = std::vector<int>{};
    result.reserve(v.size());
    auto m = std::unordered_map<int, int>{};
    for (const auto& elem : v)
    {
        auto key = static_cast<int>(elem * EPS1);
        auto [it, ok] = m.try_emplace(key, 1);
        if (!ok) m[key]++;
        result.push_back(key);
    }
    for (size_t i = 0; i < result.size(); i++)
    {
        result[i] = m[result[i]];
    }
    return result;
}

int main()
{
    auto a = std::vector<double>{ 4.0, 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 5.0, 5.1, 5.2 };
    auto b = find_freq(a);
    out(a);
    out(b);
}
