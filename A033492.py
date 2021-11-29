# https://oeis.org/A033492/list
# 54      686
# 55      689
# 57      692
# 58      705
# 59      706
# 60      745
# 61      950


from colorama import Fore, init as colorama_init


def forever(acc):
    while True:
        yield acc
        acc += 1


def flatten(a, b):
    tail, mem = b
    flat = [a]
    while tail:
        t1 = mem[tail]
        flat.append(tail)
        tail = t1
    return flat


def hailstone(n, m):
    nn = 3*n + 1 if n & 1 else n//2
    if nn not in m:
        rn = nn
        while rn > 1:
            k = 3*rn + 1 if rn & 1 else rn//2
            if rn in m:
                break
            m[rn] = k
            rn = k
    m[n] = nn
    return m[n]


class seq:
    def __init__(self, a):
        self.a = a
        self.mem = {1: None, 2: 1}

    def __iter__(self):
        while True:
            if self.a in self.mem:
                h = self.mem[self.a]
            else:
                h = hailstone(self.a, self.mem)
            yield h, self.mem
            self.a += 1


colorama_init(autoreset=True)
prev = 0
for a, b in zip(forever(1), seq(1)):
    flat = flatten(a, b)
    if (c := len(flat)) > prev:
        print(Fore.CYAN + f"[{c:>10}]", end=" ")
        print(flat, end="\n\n")
        prev = c
