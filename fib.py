def forever(acc):
    while True:
        yield acc
        acc += 1


def take(n, it):
    for a, b in zip(range(n), it):
        yield b


# fib = 0 : 1 : zipWith (+) fib (tail fib)
class fib:
    def __init__(self):
        self.a = self.b = None

    def __iter__(self):
        for value in self.values():
            self.a, self.b = self.b, value
            yield value

    def values(self):
        yield 0
        yield 1
        while True:
            yield self.a + self.b


print(list(take(100, fib())))

for a, b in zip(forever(0), fib()):
    print(f"fib({a}) = {b}")
