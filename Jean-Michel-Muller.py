"""
    f(y,z) = 108 – ( 815 – 1500/z )/y
    x(0) = 4
    x(1) = 4.25
    x(n) = f(x(n-1), x(n-2))
    You can not find x(80) without Rational/Fraction
    https://scipython.com/blog/mullers-recurrence/
    https://people.eecs.berkeley.edu/~wkahan/Mindless.pdf
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
        yield Fraction(4, 1)
        yield Fraction(17, 4)
        while True:
            def f(y, z):
                r = Fraction(815, 1) - Fraction(1500, 1) / z
                return Fraction(108, 1) - r / y
            yield f(self.b, self.a)


print(list(take(10, JMM())))

with precision(9000):
    for a, b in zip(forever(0), JMM()):
        fnumerator   = BigFloat(b.numerator)
        fdenominator = BigFloat(b.denominator)
        big = fnumerator / fdenominator
        strbig = str(big)[:140]
        print(f"{a:>5}: {strbig} {b}")
