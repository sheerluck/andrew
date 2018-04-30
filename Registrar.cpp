// http://www.nirfriedman.com/2018/04/29/unforgettable-factory/

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include <cstdlib>
#include <cxxabi.h>

std::string demangle(const char *name)
{
  int status = -4; // some arbitrary value to eliminate the compiler warning

  std::unique_ptr<char, void (*)(void *)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

  auto result = (status == 0) ? res.get() : name;
  std::cout << "status" << status << '[' << name << "->" << result << "]\n";
  return result;
}

template <class Base, class... Args>
class Factory
{
public:

  template <class ... T>
  static std::unique_ptr<Base> make(const std::string &s, T&&... args)
  {
      std::cout << "make data[" << s << "]\n";
      return data().at(s)(std::forward<T>(args)...);
  }

  template <class T>
  struct Registrar : Base
  {
    friend T;

    static bool registerT()
    {
      //const auto name = std::string{typeid(T).name()};
      const auto name = demangle(typeid(T).name());
      std::cout << "!!!" << name << '\n';
      std::cout << "registerT data\n";
      Factory::data()[name] = [](Args... args) -> std::unique_ptr<Base>
      {
        return std::make_unique<T>(std::forward<Args>(args)...);
      };
      return true;
    }
    static bool registered;

  private:
    Registrar() : Base(Key{}) { (void)registered; }
  };

  friend Base;
  virtual ~Factory(){}

private:
  class Key
  {
    Key(){};
    template <class T>
    friend
    struct Registrar;
  };
  using FuncType = std::unique_ptr<Base> (*)(Args...);
  Factory() = default;

  static auto &data()
  {
    static std::unordered_map<std::string, FuncType> s;
    for (const auto [a, b]: s)
    {
        std::cout <<   "a=" << a
                  << ", b=" << b << '\n';
    }
    std::cout << "=========================" << '\n';
    return s;
  }
};

template <class Base, class... Args>
template <class T>
bool Factory<Base, Args...>::Registrar<T>::registered =
     Factory<Base, Args...>::Registrar<T>::registerT();

struct Animal : Factory<Animal, int>
{
  Animal(Key) {}
  virtual ~Animal() {}
  virtual void makeNoise() = 0;
};

class Dog : public Animal::Registrar<Dog>
{
public:
  Dog(int x) : m_x(x) {}

  void makeNoise() { std::cerr << "Dog: " << m_x << "\n"; }

private:
  int m_x;
};

class Cat : public Animal::Registrar<Cat>
{
public:
  Cat(int x) : m_x(x) {}

  void makeNoise() { std::cerr << "Cat: " << m_x << "\n"; }

private:
  int m_x;
};

// Won't compile because of the private CRTP constructor
// class Spider : public Animal::Registrar<Cat> {
// public:
//     Spider(int x) : m_x(x) {}

//     void makeNoise() { std::cerr << "Spider: " << m_x << "\n"; }

// private:
//     int m_x;
// };

// Won't compile because of the pass key idiom
// class Zob : public Animal {
// public:
//     Zob(int x) : Animal({}), m_x(x) {}

//      void makeNoise() { std::cerr << "Zob: " << m_x << "\n"; }
//     std::unique_ptr<Animal> clone() const { return
//     std::make_unique<Zob>(*this); }

// private:
//      int m_x;

// };

// An example that shows that rvalues are handled correctly, and
// that this all works with move only types

struct Creature : Factory<Creature, std::unique_ptr<int>>
{
  Creature(Key) {}
  virtual ~Creature() {}
  virtual void makeNoise() = 0;
};

class Ghost : public Creature::Registrar<Ghost>
{
public:
    Ghost(std::unique_ptr<int>&& x) : m_x(*x) {}

    void makeNoise() { std::cerr << "Ghost: " << m_x << "\n"; }

private:
    int m_x;
};

int main() {
  auto x = Animal::make("Dog", 3);
  auto y = Animal::make("Cat", 2);
  auto z1 = Creature::make("Ghost", std::make_unique<int>(41));
  auto z2 = Ghost::make("Ghost", std::make_unique<int>(42));
  x->makeNoise();
  y->makeNoise();
  z1->makeNoise();
  z2->makeNoise();
  return 0;
}
