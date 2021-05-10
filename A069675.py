from sympy import isprime
for b in range(23, 123):
    for a in range(1, 10):
        for c in [1, 3, 7, 9]:
            if isprime(num := a * 10**b + c):
                print (num)
