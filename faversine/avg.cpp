#include "avg.h"

float
avg(const model::VF& a)
{
    const auto size = a.size();
    auto acc = float{0.f};
    for (const auto& t : a) acc += t;
    return acc/size;
}

