#include <type_traits>
#include <iostream>
#include <ostream>
#include <tuple>


template <typename TupleT,
          std::size_t... Is>
std::ostream& print_tuple_imp(std::ostream& os,
                              const TupleT& tp,
                              std::index_sequence<Is...>)
{
    size_t index = 0;
    auto print = [&index, &os]<typename T>(const T& x)
    {
        if (index++ > 0)
            os << ", ";
        if constexpr (std::is_same_v<T, char>)
        {
            os << '\'' << x << '\'';
        }
        else if constexpr (std::is_same_v<T, std::string>
                       or  std::is_same_v<T, const char*>)
        {
            os << '"' << x << '"';
        }
        else
        {
            os << x;
        }
    };

    os << "(";
    (print(std::get<Is>(tp)), ...);
    os << ")";
    return os;
}

template <typename TupleT,
         std::size_t TupleSize = std::tuple_size<std::decay_t<TupleT>>::value>
std::ostream& operator <<(std::ostream& os,
                          const TupleT& tp)
{
    return print_tuple_imp(os,
                           tp,
                           std::make_index_sequence<TupleSize>{});
}

int main()
{
    auto t = std::tuple{"hello", 1, '2', std::string{"2+2"}, 3.14, -42, "hop"};
    std::cout << t << '\n';
}
