"""
    f(y,z) = 111 – 1130/y + 3000/zy
    x(0) = 2
    x(1) = -4
    x(n) = f(x(n-1), x(n-2))
    Handbook of Floating-Point Arithmetic
      by Jean-Michel Muller,
         Nicolas Brisebarre,
         Florent de Dinechin,
         Claude-Pierre Jeannerod,
         Vincent Lefèvre,
         Guillaume Melquiond,
         Nathalie Revol,
         Damien Stehlé,
         Serge Torres
    https://github.com/mdickinson/bigfloat
"""

from fractions import Fraction
from bigfloat import BigFloat, precision


def forever(acc):
    while True:
        yield acc
        acc += 1


def take(n, it):
    for a, b in zip(range(n), it):
        yield b


# f(y,z) = 108 – ( 815 – 1500/z )/y
class JMM:
    def __init__(self):
        self.a = self.b = None

    def __iter__(self):
        for value in self.values():
            self.a, self.b = self.b, value
            yield value

    def values(self):
        yield Fraction(2, 1)
        yield Fraction(-4, 1)
        while True:
            def f(y, z):
                a = Fraction(111, 1)
                b = Fraction(1130, 1) / y
                c = Fraction(3000, 1) / (z * y)
                return a - b + c
            yield f(self.b, self.a)


print(list(take(10, JMM())))

with precision(9000):
    for a, b in zip(forever(0), JMM()):
        fnumerator   = BigFloat(b.numerator)
        fdenominator = BigFloat(b.denominator)
        big = fnumerator / fdenominator
        strbig = str(big)[:140]
        print(f"{a:>5}: {strbig} {b}")
