from math import floor, sqrt

def pair(x: int, y: int) -> int:
    if x >= y:
        return x * x + x + y
    return y * y + x


def unpair(z: int) -> tuple[int, int]:
    fs = floor(sqrt(z))
    sq = fs * fs
    if z - sq >= fs:
        return (fs, z - sq - fs)
    return (z - sq, fs)


import pytest
testdata = (
    (2818, 1412, 7945354),
    ( 283, 2592, 6718747),
    (1772, 1764, 3143520),
    (1985, 2993, 8960034),
    (1544, 1762, 3106188),
    (1178, 2501, 6256179),
    (1096, 2320, 5383496),
    (2955, 2518, 8737498),
    (  44,    0,    1980),
    (2788, 2045, 7777777),
    (1421, 1338, 2022000),
    ( 610,  452,  373162),
    ( 969,   41,  939971),
    (2072,    1, 4295257),
    (  81, 2574, 6625557),
  )


@pytest.mark.parametrize(
  ("x", "y", "expected"),
  testdata
)
def test_pair(x, y, expected):
    assert pair(x, y) == expected


@pytest.mark.parametrize(
  ("x", "y", "expected"),
  testdata
)
def test_unpair(x, y, expected):
    assert unpair(expected) == (x, y)
