#pragma once

#include "model.h"

namespace extraction {

model::VDTMLT
locateTemperatures(const int year,
                   const SV stationsFileName,
                   const SV temperaturesFileName);
model::VMLT
locationYearlyAverageRecords(const model::VDTMLT);

}

