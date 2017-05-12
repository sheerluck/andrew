#pragma once

#include <tuple>
#include <QtGlobal>

quint32
to32(const qint16 x,
     const qint16 y);


std::tuple<qint16, qint16>
from(const quint32 z);
