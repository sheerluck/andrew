from itertools import count
for n in count():
    print(n.bit_count() % 2, end="")
    if n > 100:
        break
