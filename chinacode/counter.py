"""

Бывают разные числа.
Вот по основанию  2:  100101010111110101101
Вот по основанию  8:  234543253452345233253
Вот по основанию 10:  1234567890
Вот по основанию 16:  450bad054
А я придумал числа по основанию 5000:  䉘䗵㷺䁡㹿㚴
А потом числа по основанию 20000: 乂蚌烘鎑諤靆


"""

import sys
from time import sleep
from random import randint
from typing import List



sep = "\u3000"  # Это китайский широкий пробел


def int2china(x: int) -> str:
    """ 123456789 -> 昼辵 """
    N = 20_000      # 40_000 - 20_000
    shift = 20_000  # china magic
    bits = []
    while x >= N:
        div, mod = x // N, x % N
        bits.append(chr(shift + mod))
        x = div
    bits.append(chr(shift + x))
    while len(bits) < 6:
        bits.append(sep)
    return ''.join(bits)


def main():

    counter = 111_111_111_111_111_111_111_111
    while True:
        print(int2china(counter)[::-1], f"  {counter:_}")
        counter += randint(2_000, 30_000)
        sleep(0.01)


if __name__ == '__main__':
    main()
