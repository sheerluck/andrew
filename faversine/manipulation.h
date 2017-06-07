#pragma once

#include "model.h"

namespace manipulation {

model::Grid
makeGrid(const model::VTLF& temperatures);

void
save2text(const int          year,
          const model::Grid& grid);

model::Grid
loadFromText(const int       year);

model::Grid
average(const model::wtf& vt);

model::Grid
deviation(const model::Grid& grid,
          const model::Grid& normals);


}
