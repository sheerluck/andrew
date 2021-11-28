from colorama import Fore, init as colorama_init

def forever(acc):
    while True:
        yield acc
        acc += 1


def hailstone(n):
    seq = [n]
    while n>1:
        n = 3*n + 1 if n & 1 else n//2
        seq.append(n)
    return seq


class seq:
    def __init__(self, a):
        self.a = a

    def __iter__(self):
        while True:
            yield hailstone(self.a)
            self.a += 1

colorama_init(autoreset=True)
prev = 0
for a, b in zip(forever(1), seq(1)):
    c = len(b)
    if c > prev:
        print(Fore.CYAN + f"[{c:>10}]", end=" ")
        print(b, end="\n\n")
        prev = c
