#include "visualisation.h"

#include <iostream>

#include "vincenty.h"
#include "functional.h"

namespace visualisation {

float
predictTemperature(model::VTLF temperatures,
                   model::Location location)
{
    {   // DEBUG START
        const auto p_y  = location.lat;
        const auto p_x  = location.lon;
        std::cout << QString("p_x=%1 p_y=%2\n").arg(p_x).arg(p_y).toStdString();
    }   // DEBUG STOP

    const auto eq = model::LocationEqual{};
    for (const auto& [Loc, t] : temperatures)
    {
        if (eq(Loc, location)) return t;
    }

    //dist = temperatures map { case(a,t) => (vincenty(a, location), t) }
    using  FF = std::tuple<float, float>;
    using VFF = std::vector<FF>;
    auto dist = VFF{};
    for (const auto& [Loc, t] : temperatures)  // over 9000 elements
    {
        // MOST TIME CONSUMING LINE OF CODE?
        dist.emplace_back(vincenty(Loc, location), t);
    }
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
    // top?  sort(dist).take(50)?  worth it?
    const auto all = allWeight(dist);  // MOST TIME CONSUMING LINE OF CODE?

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
                   float value)
{
    for (const auto& [t, c] : colortable)
    {
        if (t == value) return c;
    }



    return {};
}


}

