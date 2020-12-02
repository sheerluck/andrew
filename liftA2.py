"""Golang can u do dis"""

import re
from itertools import product
from inspect   import signature
from typing    import Callable


#   Just :: a -> Maybe a
def Just(x):     return {"type": "Maybe", "Nothing": False, "Just": x}


#   Nothing :: Maybe a
def Nothing():   return {"type": "Maybe", "Nothing": True}


#   cons :: a -> [a] -> [a]
def cons(x):     return lambda xs: [x] + xs

#   curry :: ((a, b) -> c) -> a -> b -> c
def curry(f):
    sp = signature(f).parameters
    if len(sp) > 1:
        return lambda x: lambda y: f(x, y)
    else:
        return f

#   typeName :: a -> String
def typeName(x):
    if isinstance(x, dict):
        return x.get('type') if 'type' in x else 'dict'
    else:
        return 'iter' if hasattr(x, '__next__') else type(x).__name__


def help_fmap(x) -> Callable:
    c = dict(list=fmapList,
             function=fmapFn,
             type=fmapFn,
             iter=fmapNext,
             tree=fmapTree,
             tuple=fmapTuple,
             Maybe=fmapMaybe)
    return c[typeName(x)]


#   fmap :: Functor f => (a -> b) -> f a -> f b
def fmap(f):     return lambda x: help_fmap(x)(f)(x)


#   fmapList :: (a -> b) -> [a] -> [b]
def fmapList(f): return lambda xs: list(map(f, xs))


#   fmapFn :: (a -> b) -> (r -> a) -> r -> b
def fmapFn(f):   return lambda g: lambda x: f(g(x))


#   fmapNext <$> :: (a -> b) -> Iter [a] -> Iter [b]
def fmapNext(f):
    def inner(x):
        while (v := next(x, None)) is not None: 
            yield f(v)
    return inner

#   fmapTree :: (a -> b) -> Tree a -> Tree b
def fmapTree(f):
    def inner(x):
        root = x['root']
        nest = [inner(v) for v in x['nest']]
        return Node(f(root))(nest)
    return inner

#   fmapTuple :: (a -> b) -> (c, a) -> (c, b)
def fmapTuple(f):
    def inner(x):
        if 2 == len(x):
            return x[0], f(x[1])
        return None  # WAT?
    return inner

#   fmapMaybe (<$>) :: (a -> b) -> Maybe a -> Maybe b
def fmapMaybe(f):
    def inner(x):
        if x["Nothing"]:
            return x
        v = f(x["Just"])
        return Just(v)
    return inner


def help_lift(x) -> Callable:
    c = dict(list=liftA2List,
             function=liftA2Fn,
             type=liftA2Fn,
             Maybe=liftA2Maybe)
    return c[typeName(x)]


#   liftA2 :: (a -> b -> c) -> f a -> f b -> f c
def liftA2(f):
    def inner(a, b):
        return help_lift(a)(f)(a)(b)
    return lambda a: lambda b: inner(a, b)


#   liftA2List :: (a -> b -> c) -> [a] -> [b] -> [c]
def liftA2List(f):
    return lambda xs: lambda ys: [
        f(*xy) for xy in product(xs, ys)
    ]


#   liftA2Fn :: (a0 -> b -> c) -> (a -> a0) -> (a -> b) -> a -> c
def liftA2Fn(op):
    def inner(f, g):
        def apply(x):
            fx = f(x)
            gx = g(x)
            return curry(op)(fx)(gx)
        return apply
    return lambda f: lambda g: inner(f, g)


#   liftA2Maybe :: (a -> b -> c) -> Maybe a -> Maybe b -> Maybe c
def liftA2Maybe(f):
    def inner(ma, mb):
        if ma["Nothing"]:
            return mb
        if mb["Nothing"]:
            return ma
        va = ma["Just"]
        vb = mb["Just"]
        v = curry(f)(va)(vb)
        return Just(v)
    return lambda ma: lambda mb: inner(ma, mb)


def help_show(x) -> Callable:
    c = dict(int=showInt,
             list=showList,
             Maybe=showMaybe)
    return c[typeName(x)]


#   show :: a -> String
def show(x):     return help_show(x)(x)


#   showInt :: Int -> String
def showInt(i):
    return f"{i}"


#   showList :: [a] -> String
def showList(xs):
    return '[' + ', '.join(show(x) for x in xs) + ']'


#   showMaybe :: Maybe a -> String
def showMaybe(x):
    if x["Nothing"]:
        return "Nothing"
    v = x["Just"]
    return f"Just {show(v)}"


# ==================================================================

#   get_all_numbers :: Maybe [Integer]
def get_all_numbers() -> dict:
    s = input("Введите что угодно с запятыми: ")
    q = list(map(int, re.findall(r'\d+', s)))
    return Just(q)


#   get_first_number :: Maybe Integer
def get_first_number() -> dict:
    s = input("Введите что угодно без запятых: ")
    if (q := re.search(r'\d+', s)):
        return Just(int(q.group()))
    return Nothing()


#   main :: IO ()
def main() -> int:
    xs = get_all_numbers()
    oh = get_first_number()
    he = show(liftA2(cons)(oh)(xs))
    print(he)
    return 0


if __name__ == "__main__":
    exit(main())
