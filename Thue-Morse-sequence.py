from itertools import count
from gmpy2 import popcount
for n in count():
    print(popcount(n) % 2, end="")
