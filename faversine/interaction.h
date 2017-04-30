#pragma once

#include <vector>

#include <QImage>

#include "model.h"

namespace interaction {

void
generateTiles(const model::VTID yearlyData);

QImage
tile(
  const model::VTLF temperatures,
  const model::VTFC colors,
  int zoom, int x, int y);

}

