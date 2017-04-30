#pragma once

#include "model.h"

namespace extraction {

model::VTQLF
locateTemperatures(const int year,
                   const SV stationsFileName,
                   const SV temperaturesFileName);
model::VTLF
locationYearlyAverageRecords(const model::VTQLF& records);

}

