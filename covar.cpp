#include <iostream>
#include <memory>
#include <vector>
#include <boost/type_index.hpp>


class Animal
{
    public:
        virtual void make_sound() const = 0;
};

class Cat : public Animal
{
    public:
        virtual void make_sound() const override
        {
            std::cout << "Nya!\n";
        }
};

class Dog : public Animal
{
    public:
        virtual void make_sound() const override
        {
            std::cout << "Wan!\n";
        }
};

class Man : public Animal
{
    public:
        virtual void make_sound() const override
        {
            std::cout << "Oi!\n";
        }
};

template<typename T>
void soundsT(const T& param)
{
    using boost::typeindex::type_id_with_cvr;
    std::cout << "T =     " 
              << type_id_with_cvr<T>().pretty_name()
              << '\n';
    std::cout << "param = " 
              << type_id_with_cvr<decltype(param)>().pretty_name()
              << '\n';

    for(const auto& elem : param) elem->make_sound();
}

void sounds(const std::vector<std::unique_ptr<Animal>>& param )
{
    for(const auto& elem : param) elem->make_sound();
    std::cout << __PRETTY_FUNCTION__
              << '\n';
}

void sounds_cat(const std::vector<std::unique_ptr<Cat>>& param )
{
    for(const auto& elem : param) elem->make_sound();
    std::cout << __PRETTY_FUNCTION__
              << '\n';
}


int main()
{
    const auto boo = []() {
        auto boo = std::vector<std::unique_ptr<Animal>>{};
        boo.push_back(std::make_unique<Cat>());
        boo.push_back(std::make_unique<Dog>());
        boo.push_back(std::make_unique<Man>());
        return boo;
    }();

    sounds(boo);

    const auto cats = []() {
        auto boo = std::vector<std::unique_ptr<Cat>>{};
        boo.push_back(std::make_unique<Cat>());
        boo.push_back(std::make_unique<Cat>());
        boo.push_back(std::make_unique<Cat>());
        return boo;
    }();

    sounds_cat(cats);

    // sounds(cats);  - no covariance!
}
