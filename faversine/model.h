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

struct Location {
    float lat;
    float lon;
};

struct Color {
    int red;
    int green;
    int blue;
};

using     FC = std::tuple<float, Color>;

using    VFC = std::vector<FC>;

using    MLT = std::tuple<           Location, float/*temperature*/>;

using   VMLT = std::vector<MLT>;

using  DTMLT = std::tuple<QDateTime, Location, float/*temperature*/>;

using VDTMLT = std::vector<DTMLT>;

using    TID = std::tuple<int, VMLT>;

using   VTID = std::vector<TID>;

using  L     = std::unordered_map<std::string, Location>;

}
