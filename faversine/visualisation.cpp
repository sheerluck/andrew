#include "visualisation.h"

#include <iostream>
#include <cmath>
#include <tuple>

#include "vincenty.h"
#include "functional.h"
#include "range.h"

namespace visualisation {

float
predictTemperature(const model::VTLF temperatures,
                   const model::Location location)
{
    const auto eq = model::LocationEqual{};
    for (const auto& [Loc, t] : temperatures)
    {
        if (eq(Loc, location)) return t;
    }

    using  FF = std::tuple<float, float>;
    using VFF = std::vector<FF>;
    auto dist = VFF{};
    for (const auto& [Loc, t] : temperatures)  // over 9000 elements
    {
        dist.emplace_back(vincenty(Loc, location), t);
    }

    // we don't need full dist, top30 is enough
    // but sorting is too expensive
    // Thank god there's std::nth_element
    const auto topN = 30;  // or 40, or 50, or 100
    std::nth_element(std::begin(dist),
                     std::begin(dist) + topN,
                     std:: end (dist),
                     [](auto& p1, auto& p2)
                     {
                         const auto& [len1, t1] = p1;
                         const auto& [len2, t2] = p2;
                         return len1 < len2;
                     }
                     );

    dist.resize(topN);

    const auto allWeight = [](VFF c)
    {
        const auto f = [](auto at)
        {
            const auto& [a, t] = at;
            // p is a positive real number, called the power parameter.
            const auto w = 1.f / (a*a); // p = 2
            return FF{w*t, w};
        };
        return fmap(f, c);
    };
    const auto all = allWeight(dist);

    auto sumwt = 0.f;
    auto sumw  = 0.f;
    for (const auto& [wt, w] : all)
    {
        sumwt += wt;
        sumw  += w;
    }
    return sumwt / sumw;  // Inverse distance weighting
}


const auto p = 255;
const auto colortable = model::VTFC
{
    { 60.f, model::Color{p,p,p}},
    { 32.f, model::Color{p,0,0}},
    { 12.f, model::Color{p,p,0}},
    {  0.f, model::Color{0,p,p}},
    {-15.f, model::Color{0,0,p}},
    {-27.f, model::Color{p,0,p}},
    {-50.f, model::Color{33,0,107}},
    {-60.f, model::Color{0,0,0}},
};
const auto last   = colortable.size() - 1;
const auto zipped = model::Zipped
{
    { 60.f, model::Color{p,p,p},     32.f, model::Color{p,0,0}},
    { 32.f, model::Color{p,0,0},     12.f, model::Color{p,p,0}},
    { 12.f, model::Color{p,p,0},      0.f, model::Color{0,p,p}},
    {  0.f, model::Color{0,p,p},    -15.f, model::Color{0,0,p}},
    {-15.f, model::Color{0,0,p},    -27.f, model::Color{p,0,p}},
    {-27.f, model::Color{p,0,p},    -50.f, model::Color{33,0,107}},
    {-50.f, model::Color{33,0,107}, -60.f, model::Color{0,0,0}},
};



model::Color
interpolateColor(  //model::VTFC colors,
                   const float value)
{
    if (const auto& [t, c] = colortable[0];    value >= t) return c;
    if (const auto& [t, c] = colortable[last]; value <= t) return c;

    for (const auto& [t, c] : colortable)
    {
        if (t == value) return c;
    }

    for (const auto& [t0, c0, t1, c1] : zipped)
    {
        if (value < t0 && value > t1)
        {
            const auto t  = (value - t0)/(t1 - t0);
            const auto x0 = std::vector<int>{c0.red, c0.green, c0.blue};
            const auto x1 = std::vector<int>{c1.red, c1.green, c1.blue};
            auto xyz = std::vector<float>{0, 0, 0};
            auto rgb = std::vector<int>  {0, 0, 0};
            for (const auto i : range(0, 3))
            {
                xyz[i] = x1[i] - x0[i];
                rgb[i] = std::lround(x0[i] + t * xyz[i]);
            }
            return {rgb[0], rgb[1], rgb[2]};
        }
    }
    return {};
}


}

