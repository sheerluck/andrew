"""

Бывают разные числа.
Вот по основанию  2:  100101010111110101101
Вот по основанию  8:  234543253452345233253
Вот по основанию 10:  1234567890
Вот по основанию 16:  450bad054
А я придумал числа по основанию 5000:  䉘䗵㷺䁡㹿㚴


Этот скрипт ожидает один параметр:
путь к обычному текстовому файлу в utf-8
"""

import sys
from typing import List


sep = "\u3000"  # Это китайский широкий пробел


def make(some: list) -> int:
    return int.from_bytes(some, byteorder='little', signed=False)


def int2china(x: int) -> str:
    """ 12345 -> 䠾䠿䡀䡁 """
    N = 5_000       # 19_000 - 14_000
    shift = 14_000  # china magic
    bits = []
    while x >= N:
        div, mod = x // N, x % N
        bits.append(chr(shift + mod))
        x = div
    bits.append(chr(shift + x))
    while len(bits) < 6:
        bits.append(sep)
    return ''.join(bits)


def pretty(ints: List[int]) -> str:
    txt: List[str] = list()
    sub: List[str] = list()
    for int_ in ints:
        str_ = int2china(int_)
        if 4 == len(sub):
            line = sep.join(sub)
            txt.append(line)
            sub = []
        sub.append(str_)
    txt.append(sep.join(sub))
    return "\n".join(txt)


def china(buffr: bytes) -> bytes:
    """
    Есть поток байт. Надо нарезать его на 8байтные куски.
    Зашифровать. Получить utf-8 текст на китайском.
    И превратить его в байты x.encode("UTF-8")
    """
    ch_: List[int] = list()
    txt: List[int] = list()
    for b in buffr:
        if 8 == len(ch_):
            txt.append(make(ch_))
            ch_ = []
        ch_.append(b)
    txt.append(make(ch_))
    return pretty(txt).encode("UTF-8")


def main():

    if 2 != len(sys.argv):
        raise Exception("Где карта, Билли? Нам нужна карта!")

    src = sys.argv[1]
    dst = src + '.䨸'

    with open(src, 'rb') as fsrc, open(dst, 'wb') as fdst:
        buf = fsrc.read()
        result = china(buf)
        fdst.write(result)


if __name__ == '__main__':
    main()
