#include "interaction.h"

#include <cmath>

#include "data.h"
#include "range.h"

namespace interaction {

void
generateTiles(const model::VTID yearlyData)
{
    auto gen = [](int year, const auto& data) {
        for (const auto z: range(0, 4))
        {
            const auto m = std::pow(2, z);
            for (const auto x: range(0, m+1))
            {
                for (const auto y: range(0, m+1))
                {
                    data::gen(year, z, x, y, data);
                }
            }
        }
    };

    for (const auto& [year, data] : yearlyData)
    {
        gen(year, data);
    }
}


QImage
tile(
  const model::VTLF temperatures,
  const model::VTFC colors,
  int zoom, int x, int y)
{
    return {};
}




}

