#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

using namespace std;

auto split = [](const string& a) -> vector<string> {
    using size   = string::size_type;
    auto found   = size{0};
    auto current = size{0};
    auto result  = vector<string>{};
    while ((found = a.find(",", current)) != string::npos) {
        result.emplace_back(a, current, found - current);
        current = found + 1;
    }
    result.emplace_back(a, current, a.size() - current);
    return result;
};

int main()
{
    auto aa = string{"2,7,3,0,5,3,5"};
    auto bb = string{"7,3,0,5,3,5,77777777777777"};
    auto ta = split(aa);
    auto tb = split(bb);

    using big = unsigned long long;
    auto add = [](const string a, const string b) {
        auto from_string = [](const string x) {
            stringstream ss(x);
            auto value = big{0};
            ss >> value;
            return value;
        };
        auto x = from_string(a);
        auto y = from_string(b);
        return x + y;
    };
    cout << add("123","321") << endl;

    auto zip = [&](const vector<string> a, const vector<string> b, decltype(add) func) {
        return 100;
    };

    cout << zip(ta, tb, add) << endl;

    /*
    cout << aa << endl;
    for( auto x: ta) cout << x << "a, ";
    cout << endl << endl << bb << endl;
    for( auto x: tb) cout << x << "b, ";
    cout << endl;
    */
}


