# https://oeis.org/A033492/list
# 54      686
# 55      689
# 57      692
# 58      705
# 59      706
# 60      745


from colorama import Fore, init as colorama_init

def forever(acc):
    while True:
        yield acc
        acc += 1


def flatten(a, b): 
    (L, tail), mem = b
    i = 1
    flat = [0] * L
    flat[0] = a
    while i < L:
        L1, t1 = mem[tail]
        flat[i] = tail
        tail = t1
        i += 1
    return flat

def hailstone(n, m):
    nn = 3*n + 1 if n & 1 else n//2
    if nn in m:
        L, tail = m[nn]
    else:
        L, tail = hailstone(nn, m)
    m[n] = (L+1, nn)
    return m[n]


class seq:
    def __init__(self, a):
        self.a = a
        self.mem = {1: (1, None), 2: (2, 1)}

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
    (c, tail), mem = b
    if c > prev:
        print(Fore.CYAN + f"[{c:>10}]", end=" ")
        print(flatten(a, b), end="\n\n")
        prev = c
