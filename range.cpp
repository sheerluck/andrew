#include <iostream>

struct range
{
    struct itertr
    {
        itertr(range* range, int index) : r{range}, i{index} {}
        itertr() : itertr{nullptr, 0} {}
        bool operator==(itertr& rhs)  { return i == rhs.i;      }
        bool operator!=(itertr& rhs)  { return !(*this == rhs); }
        int  operator*()              { return r->start + i;    }
        auto operator++()->itertr&    { ++i; return *this;      }
    private:
        range const * r;
        int i;
    };

    range(int strt, int stp) : start{strt}, count{stp-strt}
                   { if (count < 0) count=0;  }
    range(int cnt) : range{0, cnt} {}
    itertr begin() { return {this, 0};        }
    itertr end()   { return {this, count};    }
private:
    int start;
    int count;
};

int main() {
    for(auto a: range(10))  std::cout << a << ", ";  // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,  
    for(auto a: range(3,7)) std::cout << a << ", ";  // 3, 4, 5, 6,   
}
