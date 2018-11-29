from fractions import Fraction
from multiprocessing import Pool


def bernoulli(n):
    A = [Fraction(0)] * (n + 1)
    for m in range(n + 1):
        A[m] = Fraction(1, m + 1)
        for j in range(m, 0, -1):
            A[j - 1] = j * (A[j - 1] - A[j])
    return A[0]


def ber(n):
    b = bernoulli(n)
    if b:
        nu, de = b.numerator, b.denominator
        try:
            nude = nu/de
        except OverflowError:
            nude = "inf"
        a, b, c = n, f"{nu}/{de}", nude
        print(f"B({a}) = {c} # {b}")


with Pool(processes=8) as pool:
    acc = 0
    while acc < 1000:
        pool.apply_async(ber, (acc, ))
        acc += 1
    pool.close()
    pool.join()
