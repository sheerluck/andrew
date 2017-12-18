def f(xa, xb, xc):
    print('\033[{a};{b};{c}m{c:>3}\033[0m'.format(a=xa,b=xb,c=xc), end="") 

def subrange():
    pairs = [(0, 20), (30, 38), (40, 48), (90, 98), (100, 112)]
    for a, b in pairs:
        for i in range(a, b):
            yield i


for i in range(49):
    for j in range(9):
        print('{a:>2},{b:>2}'.format(a=i,b=j), end=": ")
        for num in subrange(): f(i, j, num)
        print()

i = 41
j = 5
print('{a:>2},{b:>2}'.format(a=i,b=j), end=": ")
for num in [0,1,2]: f(i, j, num)
print()

def red(text):
    return f"\033[41;5;1m{text}\033[0m"

print("WRNING")
print(red("WRNING"))
