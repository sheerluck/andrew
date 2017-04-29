#pragma once

#include <experimental/string_view>
#include <tuple>
#include <vector>

#include <QDateTime>

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

using     SV = std::experimental::string_view;

using     FC = std::tuple<float, Color>;

using    VFC = std::vector<FC>;

using    MLT = std::tuple<           model::Location, float/*temperature*/>;

using   VMLT = std::vector<MLT>;

using  DTMLT = std::tuple<QDateTime, model::Location, float/*temperature*/>;

using VDTMLT = std::vector<DTMLT>;

using    TID = std::tuple<int, VMLT>;

using   VTID = std::vector<TID>;

}
