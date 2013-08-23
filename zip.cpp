#include <iostream>
#include <string>
#include <deque>

using namespace std;

using container = deque<string>;
auto split = [](string&& a) -> container {
    auto result  = container{};
    string::size_type found, current = 0;
    while ((found = a.find(",", current)) != string::npos) {
        result.emplace_back(a, current, found - current);
        current = found + 1;
    }
    result.emplace_back(a, current, a.size() - current);
    return result;
};

using ziptainer = deque<pair<string,string>>;
auto zip = [](container&& a, container&& b) -> ziptainer {
    auto result = ziptainer{};
    auto pop = [](container& x) { auto h = x.front(); x.pop_front(); return h; };
    while (!a.empty() && !b.empty()) result.push_back( make_pair(pop(a), pop(b)) );
    return result;
};

int main()
{
    cout << "[";
    for(auto p: zip(split("2,7,3,0,5,3,5"), split("7,3,0,5,3,5,77777777777777"))) {
        cout << stoull(p.first) + stoull(p.second) << ", ";
    }
    cout << "]\n";
}


