#include "bits.h"

quint32
to32(const qint16 x,
     const qint16 y)
{
    quint32 z = x;
    z <<= 16;
    return z | (0xFFFF & y);
}

std::tuple<qint16, qint16>
from(const quint32 z)
{
    qint16 b = 0xFFFF & z;
    qint16 a = z >> 16;
    return std::tuple{a, b};
}

