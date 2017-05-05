#include "interaction.h"

#include <cmath>

#include <QtMath>
#include <QColor>
#include <QtConcurrent>

#include "data.h"
#include "visualisation.h"
#include "range.h"
#include "functional.h"

namespace interaction {

void
generateTiles(const model::VTID yearlyData)
{
    auto gen = [](int ayear, const auto& adata)
    {
        for (const auto z: range(0, 4))
        {
            const auto m = std::pow(2, z);
            for (const auto x: range(0, m))
            {
                for (const auto y: range(0, m))
                {
                    data::gen(ayear, z, x, y, adata);
                }
            }
        }
    };

    for (const auto& [year, data] : yearlyData)
    {
        gen(year, data);
    }
}


model::Location
tileLocation(int zoom, int x, int y)
{
    const auto pi  = float{3.1415926535897932384626433832795};
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
  int zoom, int x, int y)
{
    const auto width  = 256;
    const auto height = 256;

    const auto divide = [](const auto h)
    {
        const auto q = h / 4;
        auto result  = model::VTII{};
        for (const auto i : {0,1,2,3})
        {
            result.emplace_back(i*q, (i+1)*q);
        }
        return result;
    };

    const auto rangez = divide(width * height);
    const auto task = [width, height,
                       zoom, x, y,           // some ugly shit
                       &rt = temperatures]
                      (const auto& pp)
    {
        const auto& [a, b] = pp;
        auto pairs  = model::VTII{};
        for (const auto ind : range(a, b))
        {
            pairs.emplace_back(ind % width, ind / width);
        }

        const auto toLoc = [width, height,
                            zoom, x, y]
                           (const auto pxpy)
        {
            const auto& [px, py] = pxpy;
            const auto  xx = x * width  + px;
            const auto  yy = y * height + py;
            const auto  z  = zoom + 8; // a subtile at a higher zoom level (256 = 2⁸).
            const auto  nw = tileLocation(z, xx,   yy);
            const auto  se = tileLocation(z, xx+1, yy+1);
            return center(nw, se);
        };

        const auto Locations   = fmap(toLoc, pairs);

        const auto predict = [rt](auto o)
        {
            return visualisation::predictTemperature(rt,     o);
        };
        const auto interpl = []  (auto o)
        {
            return visualisation::interpolateColor  (/*rc,*/ o);
        };
        const auto temperature = fmap(predict, Locations);
        const auto colors      = fmap(interpl, temperature);
        return colors;
    };

    // God damn QtConcurrent for forcing me to write this
    using TASK  = std::function<model::VC(model::TII)>; // auto my ass
    auto future = QtConcurrent::mapped(rangez, TASK{ task });

    auto  img   = QImage{width, height, QImage::Format_ARGB32};
    QRgb* bit   = reinterpret_cast<QRgb*>(img.bits());
    auto ind    = int{0};
    auto colors = model::VC{};
    for (const auto& slice : future.results())
    {
        for (const auto& c : slice)
        {
            const auto& [r,g,b] = std::make_tuple(c.red, c.green, c.blue);
            const auto pixel = qRgba(r, g, b, 127);  // alpha
            bit[ind] = pixel; // QImage.setPixel is too slow
            ind += 1;  // coz it's future.results(), not range(0, width * height)
        }
    }
    return img;
}




}

