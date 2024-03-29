A001105 = [2,  8, 18,  32,  50,  72,  98, 128, 162, 200, 242,  288,  338,  392,  450,  512]
A005843 = [2,  4,  6,   8,  10,  12,  14,  16,  18,  20,  22,   24,   26,   28,   30,   32]
A139098 = [8, 32, 72, 128, 200, 288, 392, 512, 648, 800, 968, 1152, 1352, 1568, 1800, 2048]

for an_2n2, an_2n, an_8n2 in zip(A001105, A005843, A139098):
    a = complex(1, an_2n2)
    b = complex(an_2n2, 1)
    c = complex(an_2n, an_2n)
    d = complex(0, an_8n2)
    if a**2 + b**2 == c**2 and c**2 == d:
        print(f"{a}² + {b}² == {c}² == {d}")
