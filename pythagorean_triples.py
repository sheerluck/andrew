def forever(acc):
    while True:
        yield acc
        acc += 1


def take(n, it):
    for a, b in zip(range(n), it):
        yield b


class triples:
    def __iter__(self):
        for z in forever(1):
            for x in range(1, z + 1):
                for y in range(x, z + 1):
                    if x*x + y*y == z*z:
                        yield x, y, z

print(list(take(100, triples())))

for a, b in zip(forever(0), triples()):
    print(f"pythagorean({a}) = {b}")
