#include <boost/range/combine.hpp>
#include <iomanip>
#include <iostream>
#include <vector>
#include <numeric>
#include <chrono>
#include <list>
#include <string>

#define val const auto
#define var auto

int main() {
    using namespace std::string_literals;

    val h = "Hello"s;
    val w = "World"s;
    val start = std::chrono::system_clock::now();
    val vector = std::vector<int>{1, 2, 3, 4, 5};
    var vect0r = std::vector<int>{10, 20, 30, 40, 50};
    vect0r.insert(std::end(vect0r),
                  std::begin(vector),
                  std::end(vector));
    val hw = h + w;

    for (val& t : boost::combine(hw, vect0r))
    {
        std::cout << "["  << t.get<0>()
                  << ", " << t.get<1>()
                  << "]\n"; 
    }
    
    val end = std::chrono::system_clock::now();
    val diff = std::chrono::duration<double>{end - start};
    std::cout << "Time: "
              << std::setprecision(25)
              << diff.count() << " s" << std::endl; 
}
