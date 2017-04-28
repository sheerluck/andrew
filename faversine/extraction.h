#pragma once

#include <string>

#include "model.h"

namespace extraction {

model::VDTMLT
locateTemperatures(int year,
                   std::string stationsFileName,
                   std::string temperaturesFileName);

}

