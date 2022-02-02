# $ python names.py | less
from unicodedata import name
for num in range(200_000):
    try:
        c = chr(num)
        n = name(c)
        jp = ""
        try:
            jp = c.encode('iso-2022-jp-2004')
        except:
            pass
    except:
        pass
    else:
        if len(jp) in [8,9]:
            print(f"{num:>8} {c}: {n:<50} || {jp}")
        else:
            print(f"{num:>8} {c}: {n}")
