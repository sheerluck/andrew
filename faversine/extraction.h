#pragma once

#include "model.h"

namespace extraction {

model::VDTMLT
locateTemperatures(const int year,
                   const model::SV stationsFileName,
                   const model::SV temperaturesFileName);
model::VMLT
locationYearlyAverageRecords(const model::VDTMLT);

}

