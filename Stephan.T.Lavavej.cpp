#include <iostream>
#include <string>
#include <memory>

namespace std {

    template<class T, class... Args> unique_ptr<T> make_unique(Args&&... args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

template<class A, class B>
struct Two {
    Two(A x, B y)
        : m_a{std::make_unique<A>(x)},
          m_b{std::make_unique<B>(y)} {}
    std::unique_ptr<A> m_a;
    std::unique_ptr<B> m_b;
};

int main() {
    using lolo = unsigned long long;
    auto one = Two<std::string,lolo>{"3 ",4};
    std::cout << "Hello " << *one.m_a << *one.m_b << "\n";
}

