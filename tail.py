def fac(n, acc):
    return acc if not n else fac(n - 1, acc * n)

try:
    x = fac(1200, 1)
    print(x)
except:
    import traceback
    print(traceback.format_exc())

def bet(func):
    def wrapper(*args):
        out = (lambda f: (lambda x: x(x))(lambda y: f(lambda *args: lambda: y(y)(*args))))(func)(*args)
        while callable(out):
            out = out()
        return out
    return wrapper

fac_tail = bet( lambda f: lambda n, acc: acc if not n else f(n - 1, acc * n) )

x = fac_tail(1200, 1)
print(x)

