#pragma once

//#include <tuple>
//#include <vector>

#include "model.h"

namespace visualisation {

float
predictTemperature(const model::VTLF temperatures,
                   const model::Location location);

model::Color
interpolateColor(  //model::VTFC colors,
                   const float value);


}

