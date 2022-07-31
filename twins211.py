from sympy import isprime
from PIL import Image


def istwin(n: int) -> bool:
    if n < 100:
        if n in [3, 5, 7, 11, 13, 17, 19, 29, 31, 41, 43, 59, 61, 71, 73]:
            return True
        return False
    if isprime(n):
        if isprime(n - 2):
            return True
        if isprime(n + 2):
            return True
    return False


def main() -> int:
    M = 211
    img = Image.new("RGB", (M, M), "white")
    pixels = img.load()
    for x in range(img.size[0]):
        for y in range(img.size[1]):
            n = M * x + y + 1
            if istwin(n):
                pixels[y, x] = (50, 50, 250)
            elif isprime(n):
                pixels[y, x] = (50, 50, 50)
    img.show()
    return 0


if __name__ == "__main__":
    exit(main())
