#include <iostream>
#include <string>
#include <memory>
#include <stack>

namespace std {
  template<class T, class... Args> unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}

class Machine;

class State {
public:
  State(Machine* machine)
    : m{machine}
  {
  }
  virtual ~State() = 0;
  virtual void eat(char c) = 0;
  void error(char c);
protected:
  Machine* m;
};

State::~State() = default;

struct STOP : public State { STOP(Machine* m) : State{m} {} void eat(char c); };
struct ERR  : public State { ERR (Machine* m) : State{m} {} void eat(char c); };
struct A    : public State { A   (Machine* m) : State{m} {} void eat(char c); };
struct B    : public State { B   (Machine* m) : State{m} {} void eat(char c); };
struct C    : public State { C   (Machine* m) : State{m} {} void eat(char c); };
struct Cc   : public State { Cc  (Machine* m) : State{m} {} void eat(char c); };

class Machine {
public:
  Machine()
    : m_STOP{std::make_unique<STOP>(this)}
    , m_ERR {std::make_unique<ERR> (this)}
    , m_A   {std::make_unique<A>   (this)}
    , m_B   {std::make_unique<B>   (this)}
    , m_C   {std::make_unique<C>   (this)}
    , m_Cc  {std::make_unique<Cc>  (this)}
  {
    m_currentState = m_A.get();
    count_a = 0;
  }
  void process(std::string input) {
    for(auto ch : input) {
        m_currentState -> eat(ch);
    }
    std::cout << "\n";
  }
  void incA()    {  count_a++;              }

  void setERR()  { m_currentState = m_ERR .get(); }
  void setSTOP() { m_currentState = m_STOP.get(); }
  void setA()    { m_currentState = m_A   .get(); }
  void setB()    { m_currentState = m_B   .get(); }
  void setC()    { m_currentState = m_C   .get(); }
  void setCc()   { m_currentState = m_Cc  .get(); }

  int getCount_a()  { return count_a; }
private:
  std::unique_ptr<STOP> m_STOP;
  std::unique_ptr<ERR>  m_ERR;
  std::unique_ptr<A>    m_A;
  std::unique_ptr<B>    m_B;
  std::unique_ptr<C>    m_C;
  std::unique_ptr<Cc>   m_Cc;

  State  *m_currentState;
  int     count_a;
};

void
State::error(char c) {
  std::cout << "Error: '" << c << "'\n";
  m->setERR();
}

void
STOP::eat(char c) {
  static auto tail = std::stack<char>{{'c','c'}};
  if (tail.size()) {
    auto a = tail.top();
    if (a == c) {
      tail.pop();
      if (!tail.size()) {
        std::cout << "{l}{m}";
        auto counter = m->getCount_a();
        for(int i=0; i<counter; i++ ) std::cout << "{k}";
        std::cout << "\n";
        m->setERR();
      }
    } else {
      error(c);
    }
  }
}

void
ERR::eat(char c) {
  static bool header = true;
  if (header) {
    header = false;
    std::cout << "unprocessed: ";
  }
  std::cout << c;
}

void
A::eat(char c) {
  m->incA();
  if ('a' == c) {
    m->setB();
  } else {
    error(c);
  }
}

void
B::eat(char c) {
  if ('b' == c) {
    std::cout << "{l}";
    m->setC();
  } else {
    if ('a' == c) {
      m->setSTOP();
    } else {
      error(c);
    }
  }  
}

void
C::eat(char c) {
  if ('c' == c) {
    m->setCc();
  } else {
    error(c);
  }
}

void
Cc::eat(char c) {
  if ('b' == c) {
    m->setB();
  } else {
    if ('a' == c) {
      std::cout << "{m}";
      m->setA();
    } else {
      error(c);
    }
  }
}

int main(void)
{
  auto m     = Machine{};
  auto input = std::string{"abcbacc"};
  std::cin >> input;
  m.process(input);
}


