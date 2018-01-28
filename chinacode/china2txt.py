"""

Бывают разные числа.
Вот по основанию  2:  100101010111110101101
Вот по основанию  8:  234543253452345233253
Вот по основанию 10:  1234567890
Вот по основанию 16:  450bad054
А я придумал числа по основанию 5000:  䉘䗵㷺䁡㹿㚴


Этот скрипт ожидает один параметр:
путь к текстовому китайскому файлу с 䨸
"""

import sys
from typing import List


sep = "\u3000"  # Это китайский широкий пробел


def make(some: int) -> bytes:
    return some.to_bytes(8, byteorder='little', signed=False)


def china2int(x: str) -> int:
    """ 䠾䠿䡀䡁 -> 12345 """
    N = 5_000       # 19_000 - 14_000
    shift = 14_000  # china magic
    boo = [N**i for i in range(6)]
    sum_ = 0
    for mul, ch_ in zip(boo, x):
        digit = ord(ch_) - shift
        sum_ += mul * digit
    return sum_


def russia(buffr: str) -> bytes:
    """
    Есть строки в текстовом файле.
    Их надо по сепаратору нарезать на куски
    Расшифровать в 8байтные целые.
    и сделать из этого поток байт
    """
    txt: List[bytes] = list()

    for line in buffr.split("\n"):
        if '' == line:
            continue
        for chunk in line.split(sep):
            if '' == chunk:
                continue
            int_ = china2int(chunk)
            txt.append(make(int_))
    result = b''.join(txt)
    return result.rstrip(b'\x00')


def main():

    if 2 != len(sys.argv):
        raise Exception("Где карта, Билли? Нам нужна карта!")

    src = sys.argv[1]
    dst = src + '.ru'

    if not src.endswith('.䨸'):
        raise Exception("Где 䨸, Билли? Нам нужна 䨸!")

    with open(src, 'r') as fsrc, open(dst, 'wb') as fdst:
        buf = fsrc.read()
        result = russia(buf)
        fdst.write(result)


if __name__ == '__main__':
    main()
