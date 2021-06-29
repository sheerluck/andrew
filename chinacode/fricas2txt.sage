"""
20760 = 0     # base = 20180
20761 = 1
...
40939 = 20179
"""

import sys


sep = "\u3000"


def fricas2int(x: str) -> int:
    """ 䠾䠿䡀䡁 -> 12345 """
    base  = 20_180  # 40940 - 20760
    shift = 20_760  # fricas magic
    boo = [base^i for i in range(8)]
    sum_ = 0
    for mul, ch_ in zip(boo, x):
        digit = ord(ch_) - shift
        sum_ += mul * digit
    return sum_


def s2b(buffr: str) -> bytes:
    def make(some: int) -> bytes:
        return int(some).to_bytes(13, byteorder='little', signed=False)
    txt: list[bytes] = list()

    for line in buffr.split("\n"):
        if '' == line:
            continue
        for chunk in line.split(sep):
            if '' == chunk:
                continue
            int_ = fricas2int(chunk)
            txt.append(make(int_))
    result = b''.join(txt)
    return result.rstrip(b'\x00')


def main():

    if 2 != len(sys.argv):
        raise Exception("filename!")

    src = sys.argv[1]
    dst = src + '.ru'

    if not src.endswith('.丠'):
        raise Exception("丠!")

    with open(src, 'r') as fsrc, open(dst, 'wb') as fdst:
        buf = fsrc.read()
        result = s2b(buf)
        fdst.write(result)


if __name__ == '__main__':
    main()
