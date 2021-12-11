# https://oeis.org/A033492/list
# 54      686
# 55      689
# 56      692
# 57      705
# 58      706
# 59      745
# 60      950


from itertools import count
from colorama import Fore, init as colorama_init


def hailstone(n, m, p):
    i = n
    seq = [n]
    future = 0
    while n > 1:
        n = 3 * n + 1 if n & 1 else n // 2
        if not future:
            if n in m:
                future = m[n] + len(seq)
                if i not in m:
                    m[i] = future
                if future <= p:
                    for L, e in zip(range(future - 1, 1, -1), seq[1:]):
                        if e in m:
                            break
                        m[e] = L
                    return m[n], []
        seq.append(n)
    ls = len(seq)
    for L, e in zip(range(ls - 1, 1, -1), seq[1:]):
        if e in m:
            break
        m[e] = L
    m[i] = ls
    return ls, seq


colorama_init(autoreset=True)
memo = {}
prev = 0
incr = 1
for n in count(1):
    if n in memo:
        continue
    L, flat = hailstone(n, memo, prev)
    if L > prev:
        print(Fore.RED + f"[{incr:>4}]", end=" ")
        print(Fore.CYAN + f"[{L:>10}]", end=" ")
        print(flat, end="\n\n")
        prev = L
        incr += 1
