#include <iostream>
#include <vector>
#include <unordered_map>

struct vec3
{
    float x;
    float y;
    float z;
};

std::ostream& operator << (std::ostream& o, const vec3& p)
{
    return o << "x: " << p.x << ", y: " << p.y << ", z: " << p.z << std::endl;
}

struct vec3Hash
{
    std::size_t operator()(const vec3& k) const
    {
        size_t h1 = std::hash<float>()(k.x);
        size_t h2 = std::hash<float>()(k.y);
        size_t h3 = std::hash<float>()(k.z);
        return (h1 ^ (h2 << 1)) ^ h3;
    }
};

struct vec3Equal
{
    bool operator()(const vec3& lhs, const vec3& rhs) const
    {
        auto dx = std::abs(lhs.x - rhs.x);
        auto dy = std::abs(lhs.y - rhs.y);
        auto dz = std::abs(lhs.z - rhs.z);
        return 0.01 > dx + dy + dz;
    }
};

using Map = std::unordered_map<vec3, int, vec3Hash, vec3Equal>;

struct index
{
    int a;
    int b;
    int c;
};

std::ostream& operator << (std::ostream& o, const index& p)
{
    return o << "a: " << p.a << ", b: " << p.b << ", c: " << p.c << std::endl;
}


int main()
{
    auto loaded = std::vector<vec3>{
        {0.0, 0.0,  0.0},  // 0   0       0
        {0.8, 0.6,  0.0},  // 1   1       1
        {1.0, 0.0,  0.0},  // 2   2       2
        {0.0, 0.0,  0.0},  // 0   3 
        {0.6, 0.8,  0.0},  // 3   4       3
        {0.8, 0.6,  0.0},  // 1   5
        {0.0, 0.0,  0.0},  // 0   6
        {0.0, 1.0,  0.0},  // 4   7       4
        {0.6, 0.8,  0.0},  // 3   8
    };

    auto f = std::vector<index> {
        {0, 1, 2},    // 0
        {3, 4, 5},    // 1
        {6, 7, 8},    // 2
    };

    for (const auto& x: f)
    {
        std::cout << "Tre: \n"
            << "1: "  << loaded[x.a] << '\n'
            << "2: "  << loaded[x.b] << '\n'
            << "3: "  << loaded[x.c] << '\n';
        std::cout << "-------\n";
    }

    auto result4vec = std::vector<vec3>{};
    result4vec.reserve(loaded.size());
    auto result4f   = std::vector<index>{};
    result4f.reserve(f.size());
    auto m = Map{};
    auto q = std::vector<int>{};
    q.reserve(loaded.size());
    int counter = 0;
    int uniq_counter = 0;
    for (const auto& x: loaded)
    {
        auto search = m.find(x);
        if (search != std::end(m)) // found
        {
            std::cout << "ДУБЛИКAТ!  " << x << '\n';
            q[counter] = m[x];
        }
        else
        {
            m[x] = uniq_counter;
            q[counter] = result4vec.size();
            result4vec.push_back(x);
            uniq_counter += 1;
        }
        counter += 1;
    }

    for (const auto& x: f)
    {
        auto newindex = index{q[x.a], q[x.b], q[x.c]};
        result4f.push_back(newindex);
    }

    std::cout << "Map:\n";
    for (const auto& [v, i]: m)
    {
        std::cout << "i=" << i
                  << "\tv=" << v
                  << '\n';
    }

    std::cout << "=====================:\n";
    std::cout << "САМОЕ КЛЮЧЕВОЕ! new f:\n";
    for (const auto& x: result4f)
    {
        std::cout << x << '\n';
    }

    std::cout << "И, наконец, результат:\n";

    for (const auto& x: result4f)
    {
        std::cout << "Tre: \n"
            << "1: "  << result4vec[x.a] << '\n'
            << "2: "  << result4vec[x.b] << '\n'
            << "3: "  << result4vec[x.c] << '\n';
        std::cout << "-------\n";
    }
}

