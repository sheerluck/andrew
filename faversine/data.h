#pragma once

#include <tuple>
#include <vector>

#include "model.h"

namespace data {

model::TID
data(int year);

void
gen(int year, int zoom,
    int x,    int y,
    model::VMLT data);


}

