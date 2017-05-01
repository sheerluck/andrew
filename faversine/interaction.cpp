#include "interaction.h"

#include <cmath>

#include <QtMath>

#include "data.h"
#include "visualisation.h"
#include "range.h"
#include "functional.h"

namespace interaction {

void
generateTiles(const model::VTID yearlyData)
{
    auto gen = [](int year, const auto& data)
    {
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
        //std::cout << data.size() << '\n';

        //for (const auto& [L, t] : data)  std::cout << L.lat << ' ' << L.lon << ' ' << t << '\n';
        gen(year, data);
    }
}


model::Location
tileLocation(int zoom, int x, int y)
{
    const auto pi  = float{3.1415926535897932384626433832795};
    const auto tor = pi / 180.f;
    const auto n   = std::pow(2.f, 1.f*zoom);
    const auto lonDeg = 1.f*x / n * 360.f - 180.f;
    const auto k      = 1.f*y / n;
    const auto latRad = std::atan(std::sinh(pi * (1.f - 2.f * k)));
    return {qRadiansToDegrees(float{latRad}), lonDeg};
}



model::Location
center(const model::Location a, const model::Location b)
{
    const auto lat = 0.5f * (a.lat + b.lat);
    const auto lon = 0.5f * (a.lon + b.lon);
    return {lat, lon};
}


QImage
tile(
  const model::VTLF temperatures,
  const model::VTFC colors,
  int zoom, int x, int y)
{
    const auto width  = 256;
    const auto height = 256;

    auto pairs  = model::VTII{};
    for (const auto ind : range(0, width * height))
    {
        pairs.emplace_back(ind % width, ind / width);
    }

    //for (const auto& [a, b] : pairs)
    //    std::cout << a << ',' << b << '\n';

    const auto toLoc = [width, height,
                        zoom, x, y]
                       (const auto pxpy)
    {
        const auto [px, py] = pxpy;
        const auto xx = x * width  + px;
        const auto yy = y * height + py;
        const auto z  = zoom + 8; // a subtile at a higher zoom level (256 = 2⁸).
        const auto nw = tileLocation(z, xx,   yy);
        const auto se = tileLocation(z, xx+1, yy+1);
        return center(nw, se);
    };

    const auto Locs    = fmap(toLoc, pairs);

    //Locs.resize(10);
    //for (const auto& elem : Locs)
    //    std::cout << elem.lat << ',' << elem.lon << '\n';

    const auto predict = [&rt = temperatures]  (auto o)
    {
        return visualisation::predictTemperature(rt, o);
    };
    const auto interpl = [&rc = colors]        (auto o)
    {
        return visualisation::interpolateColor  (rc, o);
    };
    const auto t = fmap(predict, Locs);
    const auto c = fmap(interpl, t   );
    return QImage{};
}




}

