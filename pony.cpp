#include <vector>
#include <iostream>

#define val const auto
#define var auto

int main()
{
    using namespace std::string_literals;

    val tests = std::vector<std::vector<std::string>>
    {
       {"123"s, "Jack"s, "Jill", "777"s},
       {"123"s, "Jack"s, "Noll", "777"s},
       {"123"s, "Nock"s, "Noll", "777"s},
       {}
    };

    for (val& test: tests)
    {
        var names = test;
        val moreNames = [&]() {return names.size() > 0;};
        val getName   = [&]() {
          val last = names.back();
          names.pop_back();
          return last;
        };
           
        val name = [&](){
          var last = "^last^"s;
          while (moreNames()) {
            val name = getName();
            if (name == "Jack"s || name == "Jill"s) return name;
            last = name;
          }
          if (last == "^last^"s) return "Herbert"s;
          return last;
        }();
 
        std::cout << "Hello, " << name << std::endl;
    }
}
