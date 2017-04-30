#pragma once

//#include <tuple>
#include <vector>

#include "model.h"

namespace data {

model::TID
data(const int year);

void
gen(const int year, const int zoom,
    const int x,    const int y,
    const model::VTLF& data);


}

