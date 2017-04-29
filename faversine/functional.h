#pragma once

#include <functional>
#include <type_traits>

template<typename F, typename T>
auto fmap(const F& f, const std::vector<T>& v) {
    using U     = std::decay_t<std::result_of_t<F(T)>>;
    auto result = std::vector<U>{};
    result.reserve(v.size());
    for(const auto& elem : v) result.emplace_back(f(elem));
    return result;
}

template<typename F, typename T>
auto fmap(const F& f, std::vector<T>&& v) {
    using U     = std::decay_t<std::result_of_t<F(T)>>;
    auto result = std::vector<U>{};
    result.reserve(v.size());
    for(const auto& elem : v) result.emplace_back(f(std::move(elem)));
    return result;
}
