#pragma once

struct range
{
    struct itertr
    {
        itertr(range* range, int index) : r{range}, i{index} {}
        bool operator!=(itertr& rhs)   { return i != rhs.i;   }
        int  operator*()               { return r->start + i; }
        auto operator++()->itertr&     { ++i; return *this;   }
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
