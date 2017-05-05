#include "vincenty.h"

#include <cmath>
#include <array>

#include "range.h"
#include "functional.h"
#include <QtMath>

constexpr auto a = 6378137.f;
constexpr auto b = 6356752.314245f;
constexpr auto ab= a + b;
constexpr auto f = 1.f / 298.257223563;
constexpr auto g = 1.f - f;
constexpr auto h = f / 16;
constexpr auto viter = 200;
constexpr auto threshold = 1e-12;

using std::abs;

float
sqr(float x)
{
    return x * x;
}


const auto sintab = []() {
    auto arr = std::array<float, 2*314160>{};
    for (const auto i : range(0, 2*314160))
    {
        auto x  = float{ -314160.f + i};
        x      /= 100000.f;
        arr[i]  = sin(x);
    }
    return arr;
}();

const auto costab = []() {
    auto arr = std::array<float, 2*314160>{};
    for (const auto i : range(0, 2*314160))
    {
        auto x  = float{ -314160.f + i};
        x      /= 100000.f;
        arr[i]  = cos(x);
    }
    return arr;
}();

const auto asintb = []() {
    auto arr = std::array<float, 2*500000 + 1>{};
    for (const auto i : range(0, 2*500000 + 1))
    {
        auto x  = float{ -500000.f + i};
        x      /= 500000.f;
        arr[i]  = asin(x);
    }
    return arr;
}();

const auto asin10 = []() {
    auto arr = std::array<float, 2*500000 + 1>{};
    for (const auto i : range(0, 2*500000 + 1))
    {
        auto x  = float{ -500000.f + i};
        x      /= 5000000.f;
        arr[i]  = asin(x);
    }
    return arr;
}();


std::string test_me_sin(const float vonny)
{
    auto ind = static_cast<int>(100000.f * vonny);
    ind     += 314160;
    auto s = QString("sin(%1)=%2, tab[%3]=%4")
                    .arg(vonny).arg(sin(vonny))
                    .arg(ind)  .arg(sintab[ind]);
    return s.toStdString();
}

std::string test_me_asin(const float vonny)
{
    auto ind = int{0};
    auto val = float{0.f};
    auto tab = "!";
    if (abs(vonny) < 0.001f)
    {
        ind = 1;
        val = vonny;
        tab = "?";
    }
    else
    if (abs(vonny) < 0.1f)
    {
        ind = static_cast<int>(5000000.f * vonny);
        ind += 500000;
        val = asin10[ind];  // -0.1 .. +0.1
        tab = "2";
    }
    else
    {
        ind = static_cast<int>(500000.f * vonny);
        ind += 500000;
        val = asintb[ind];  // -1 .. +1
        tab = "1";
    }
    auto s = QString("asin(%1)=%2, tab%3[%4]=%5")
                    .arg(vonny).arg(asin(vonny))
                    .arg(tab)
                    .arg(ind)  .arg(val);
    return s.toStdString();
}


float
haversine(const std::vector<float>&& p)
{
    const auto r    = fmap([](float x){ return qDegreesToRadians(x); }, p);
    const auto dlat = r[2] - r[0];
    const auto dlon = r[3] - r[1];
    const auto slat = sin(0.5f  * dlat);
    const auto slon = sin(0.5f  * dlon);
    const auto k    = cos(r[0]) * cos(r[2]);
    const auto d    = sqr(slat) + k * sqr(slon);
    return ab * asin(sqrt(d));
}

float
vincenty(const model::Location x,
         const model::Location y)
{
    const auto p1_0 = x.lat;
    const auto p1_1 = x.lon;
    const auto p2_0 = y.lat;
    const auto p2_1 = y.lon;
    return haversine(std::vector<float>{p1_0, p1_1, p2_0, p2_1});/*
    if (p1_0 == p2_0 && p1_1 == p2_1) return 0.f;
    if (abs(p2_0 - p1_0) > 80)        return haversine(std::vector<float>{p1_0, p1_1, p2_0, p2_1});
    if (abs(p2_1 - p1_1) > 80)        return haversine(std::vector<float>{p1_0, p1_1, p2_0, p2_1});
    const auto u1 = atan(g * tan(qDegreesToRadians(p1_0)));
    const auto u2 = atan(g * tan(qDegreesToRadians(p2_0)));
    const auto L  = qDegreesToRadians(p2_1 - p1_1);
    / V / auto lambda = L;
    const auto sinU1 = sin(u1);
    const auto cosU1 = cos(u1);
    const auto sinU2 = sin(u2);
    const auto cosU2 = cos(u2);

    auto sinSigma    = 0.f;
    auto cosSigma    = 0.f;
    auto sigma       = 0.f;
    auto cosSqAlpha  = 0.f;
    auto cos2SigmaM  = 0.f;

    for ( [[maybe_unused]] const auto iteration : range(0, viter))
    {
        const auto sinLambda = sin(lambda);
        const auto cosLambda = cos(lambda);
        const auto q1 = cosU2 * sinLambda;
        const auto q2 = cosU1 * sinU2 - sinU1 * cosU2 * cosLambda;
        sinSigma = sqrt(sqr(q1) + sqr(q2));
        if (0.f == sinSigma) break;
        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma    = atan2(sinSigma, cosSigma);
        const auto sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
        cosSqAlpha = 1.f - sqr(sinAlpha);
        cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;
        if (std::isnan(cos2SigmaM)) cos2SigmaM = 0.f;
        if (std::isinf(cos2SigmaM)) cos2SigmaM = 0.f;
        const auto C = h * cosSqAlpha * (4 + f * (4 - 3 * cosSqAlpha));
        const auto lambdaPrev = lambda;
        const auto k1 = f * (1.f - C) * sinAlpha;
        const auto k2 =            C  * sinSigma;
        const auto k3 =            C  * cosSigma;
        const auto k4 = -1.f + 2.f * sqr(cos2SigmaM);
        lambda = L + k1 * ( sigma + k2 * ( cos2SigmaM + k3 * k4 ));
        if (abs(lambda - lambdaPrev) < threshold) break;
    }

    if (0.f == sinSigma) return 0.f;
    const auto k   = sqr(a) - sqr(b);
    const auto uSq = k * cosSqAlpha / sqr(b);
    const auto A  = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
    const auto B  =     uSq /  1024 * ( 256 + uSq * (-128 + uSq * ( 74 -  47 * uSq)));
    const auto B4 = B / 4.f;
    const auto B6 = B / 6.f;
    const auto k1 = B * sinSigma;
    const auto k2 = -1.f + 2.f * sqr(cos2SigmaM);
    const auto k3 = -3.f + 4.f * sqr(sinSigma);
    const auto k4 = -3.f + 4.f * sqr(cos2SigmaM);
    const auto dSigma = k1 * (cos2SigmaM + B4 * (cosSigma * k2 - B6 * cos2SigmaM * k3 * k4));
    return b * A * (sigma - dSigma);*/
}

