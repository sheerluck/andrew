from fractions import Fraction


def take(n, it):
    for a, b in zip(range(n), it):
        yield b


def bernoulli(n):
    A = [Fraction(0)] * (n + 1)
    for m in range(n + 1):
        A[m] = Fraction(1, m + 1)
        for j in range(m, 0, -1):
            A[j - 1] = j * (A[j - 1] - A[j])
    return A[0]


def ber(n):
    while True:
        b = bernoulli(n)
        if b:
            nu, de = b.numerator, b.denominator
            try:
                nude = nu/de
            except OverflowError:
                nude = "inf"
            yield n, f"{nu}/{de}", nude
        n += 1


print(list(take(100, ber(0))))

for a, b, c in ber(0):
    print(f"B({a}) = {c} # {b}")
