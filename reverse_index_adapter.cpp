#include <iostream>
#include <vector>

template<typename T>
struct reverse_adapter
{
    reverse_adapter(T& acontainer)
        : container{acontainer}
    {}

    decltype(auto) begin()
    {
        return container.rbegin();
    }

    decltype(auto) end()
    {
        return container.rend();
    }
    T& container;
};

template<typename T>
struct reverse_index_adapter
{
    struct iterator
    {
        typename T::reverse_iterator iter;
        size_t index;

        auto operator*()
        {
            return std::pair(*iter, index);
        }

        auto operator++() -> iterator&
        {
            ++iter;
            --index;
            return *this;
        }

        bool operator==(const iterator& other) const
        {
            return iter == other.iter;
        }

        bool operator!=(const iterator& other) const
        {
            return iter != other.iter;
        }
    };

    reverse_index_adapter(T& acontainer)
        : container{acontainer}
    {}

    iterator begin()
    {
        return {container.rbegin(), container.size()-1};
    }

    iterator end()
    {
        return {container.rend(), 0};
    }
    T& container;
};

int main()
{
    auto vec = std::vector<int>{4, 22, 0};
    for (const auto& value : vec)
    {
        std::cout << "v=" << value
                  << '\n';
    }
    std::cout << '\n';
    for (const auto& value : reverse_adapter(vec))
    {
        std::cout << "v=" << value
                  << '\n';
    }
    std::cout << '\n';
    for (const auto& [value, idx] : reverse_index_adapter(vec))
    {
        std::cout << "v=" << value
                  << ", i=" << idx
                  << '\n';
    }
    std::cout << '\n';
}


