#include <iostream>
#include <limits>
#include <variant>
#include <map>
#include <charconv>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>

[[nodiscard]]
__int128_t 
atoint128_t(std::string_view sv)
{
    auto n = __int128_t{0};
    auto sign = false;

    if (sv.starts_with('-'))
    {
        sv.remove_prefix(1);
        sign = true;
    }

    if (sv.starts_with('+'))
    {
        sv.remove_prefix(1);
    }

    for(const char c : sv)
    {
        if (not std::isdigit(c)) break;
        n *= 10;
        n += c - '0';
    }

    return sign ? -n : n;
}

[[nodiscard]]
std::string
to_string(const __int128_t& x)
{
    using namespace std::string_literals;
    if (x <  0) return "-"s + to_string(-x);
    if (x < 10) return ""s + (char)(x + '0');
    const auto [a, b] = std::tuple{x / 10, x % 10};
    return to_string(a) + (char)(b + '0');
}


template <typename T>
concept is_from_chars_convertible = requires (const char* first,
                                              const char* last,
                                              T& out) {
    std::from_chars(first, last, out);
};

template <typename T>
[[nodiscard]]
std::optional<T>
try_convert(std::string_view sv) noexcept
{
    auto value = T{};
    if constexpr (is_from_chars_convertible<T>)
    {
        const auto last = sv.data() + sv.size();
        const auto res  = std::from_chars(sv.data(), last, value);
	if (res.ec == std::errc{} and res.ptr == last)
            return value;
    }
    else
    {
        try
        {
            const auto str = std::string{sv};
            auto read = std::size_t{0};

            if constexpr (std::is_same_v<T, double>)
            {
                value = std::stod(str, &read);
                if (str.size() == read) return value;
            }

            if constexpr (std::is_same_v<T, float>)
            {
                value = std::stof(str, &read);
                if (str.size() == read) return value;
            }

            if constexpr (std::is_same_v<T, __int128_t>)
            {
                return atoint128_t(str);
            }
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << '\n';
        }
    }
    return std::nullopt;
}

int main()
{
    using uint64 = std::uint64_t;
    using int128 = __int128_t;
    using lngdbl = long double;

    if (const auto val = try_convert<uint64>("2021"); val)
        std::cout << val.value_or(-1)
                  << " uint64 ok\n";

    if (const auto val = try_convert<int128>("-777"); val)
        std::cout << to_string(val.value_or(-1))
                  << " int128 ok\n";

    if (const auto val = try_convert<float> ("19.7"); val)
        std::cout << val.value_or(-1)
                  << " float  ok\n";

    if (const auto val = try_convert<double>("3.14"); val)
        std::cout << val.value_or(-1)
                  << " double ok\n";

    if (const auto val = try_convert<lngdbl>("0.11"); val)
        std::cout << val.value_or(-1)
                  << " lngdbl ok\n";
}
