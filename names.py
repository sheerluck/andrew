# $ python names.py | less
from unicodedata import name
for num in range(131_000):
    try:
        c = chr(num)
        n = name(c)
    except:
        pass
    else:
        print(f"{num:>8} {c}: {n}")
