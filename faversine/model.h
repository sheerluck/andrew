#pragma once

#include <experimental/string_view>
#include <experimental/optional>
#include <tuple>
#include <vector>
#include <unordered_map>

#include <QDateTime>

using  SV = std::experimental::string_view;

template<typename T>
using Opt = std::experimental::optional<T>;

namespace model {

struct Location
{
    float lat;
    float lon;
};

struct LocationHash
{
    std::size_t operator()(const Location& k) const
    {
        union cast
        {
            float  input[2];
            double result;
        } d;
        d.input[0] = k.lat;
        d.input[1] = k.lon;
        return std::hash<double>()(d.result);
    }
};

struct LocationEqual
{
    bool operator()(const Location& lhs, const Location& rhs) const
    {
        return lhs.lat == rhs.lat && lhs.lon == rhs.lon;
    }
};

struct Color
{
    int red;
    int green;
    int blue;
};

using     VC = std::vector<Color>;

using    TFC = std::tuple<float, Color>;

using   VTFC = std::vector<TFC>;

using   Four = std::tuple<float, Color, float, Color>;

using Zipped = std::vector<Four>;

using    TLF = std::tuple<           Location, float/*temperature*/>;

using   VTLF = std::vector<TLF>;

using   TQLF = std::tuple<QDateTime, Location, float/*temperature*/>;

using  VTQLF = std::vector<TQLF>;

using    TID = std::tuple<int, VTLF>;

using   VTID = std::vector<TID>;

using    MSL = std::unordered_map<std::string, Location>;

using  MLVFh = std::unordered_map<Location, std::vector<float>, LocationHash, LocationEqual>;

using    TII = std::tuple<int, int>;

using   VTII = std::vector<TII>;


}
