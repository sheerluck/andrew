#pragma once

//#include <tuple>
//#include <vector>

#include "model.h"

namespace visualisation {

float
predictTemperature(model::VTLF temperatures,
                   model::Location location);

model::Color
interpolateColor(  model::VTFC colors,
                   float value);


}

