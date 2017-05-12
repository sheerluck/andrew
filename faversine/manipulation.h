#pragma once

#include "model.h"

namespace manipulation {

// def makeGrid(temperatures: Iterable[(Location, Double)]): (Int, Int) => Double = {

model::Grid
makeGrid(const model::VTLF& temperatures);

void
save2text(const int          year,
          const model::Grid& grid);

}
