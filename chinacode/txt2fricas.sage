"""
20760 = 0     # base = 20180
20761 = 1
...
40939 = 20179
"""

import sys


sep = "\u3000"


def int2fricas(x: int) -> str:
    """ 12345 -> 䠾䠿䡀䡁 """
    base  = 20_180  # 40940 - 20760
    shift = 20_760  # fricas magic
    bits  = []
    txt   = f"radix({x}, {base})"
    for mod in reversed(str(fricas(txt)).split()):
        bits.append(chr(shift + int(mod)))
    while len(bits) < 8:
        bits.append(sep)
    return ''.join(bits)


def pretty(ints: list[int]) -> str:
    txt: list[str] = list()
    sub: list[str] = list()
    for int_ in ints:
        str_ = int2fricas(int_)
        if 4 == len(sub):
            line = sep.join(sub)
            txt.append(line)
            sub = []
        sub.append(str_)
    txt.append(sep.join(sub))
    return "\n".join(txt)


def b2s(buffr: bytes) -> bytes:
    def make(some: list[int]) -> int:
        return int.from_bytes(some, byteorder='little', signed=False)
    ch_: list[int] = list()
    txt: list[int] = list()
    for b in buffr:
        if 13 == len(ch_):
            txt.append(make(ch_))
            ch_ = []
        ch_.append(b)
    txt.append(make(ch_))
    return pretty(txt).encode("utf-8")


def main():

    if 2 != len(sys.argv):
        raise Exception("filename!")

    src = sys.argv[1]
    dst = src + '.丠'

    with open(src, 'rb') as fsrc, open(dst, 'wb') as fdst:
        buf = fsrc.read()
        result = b2s(buf)
        fdst.write(result)


if __name__ == '__main__':
    main()
