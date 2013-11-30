from inspect import getsource as g
m = "\033[1;%sm%s\033[1;m"
a = lambda t: m % (n, t)
f = lambda q: "\n" if q in [3, 7, -7] else ""
p = lambda q: print(a(g(a)[: -1]), end=f(q))
for n in [0, 4, 5, 7]: p(-n)
for k in [3, 4]:
    for n in range(10 * k, 10 * k + 8): p(n % 10)
