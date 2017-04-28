#pragma once

#include <vector>

#include <QImage>

#include "model.h"

namespace interaction {

void
generateTiles(const model::VTID yearlyData);

QImage
tile(
  const model::VMLT temperatures,
  const model::VFC  colors,
  int zoom, int x, int y);

}

