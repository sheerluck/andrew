from collections import namedtuple
from math import sin, pi
from fn import _

Tree = namedtuple("Tree", ['f', 'L', 'R'])
bold = lambda t: "\033[1;%sm%s\033[1;m" % (37, t)
leaf = lambda x: Tree(f=lambda a, b: x, L=None, R=None)
tour = lambda t: t.f(tour(t.L), tour(t.R)) if t else None
whch = lambda t, x: t.L if "L" == x else t.R
subt = lambda t, s: subt(whch(t, s[:1]), s[1:]) if s else t
newt = lambda x, t, s, c: plac(x, t, s[1:]) if c == s[:1] else t
plac = lambda x, t, s: Tree(f=t.f, L=newt(x, t.L, s, "L"), R=newt(x, t.R, s, "R")) if s else x

tree = Tree(f=_ * _,
            L=Tree(f=_ + _,
                   L=leaf(pi),
                   R=leaf(pi / 2)
                   ),
            R=Tree(f=_ / _,
                   L=Tree(f=lambda a, b: sin(a if a else b),
                          L=None,
                          R=leaf(pi / 4)
                          ),
                   R=Tree(f=lambda a, b: sin(a if a else b),
                          L=leaf(pi / 3),
                          R=None
                          )
                   ),
            )
print(tree,   "\n", bold(tour(tree)), "\n")

branch = subt(tree, "RR")
print(branch, "\n", bold(tour(branch)), "\n")

branch = plac(leaf(pi + pi / 2), tree, "L")
print(branch, "\n", bold(tour(branch)), "\n")

branch = plac(subt(tree, "L"), plac(subt(tree, "R"), tree, "L"), "R")
print(branch, "\n", bold(tour(branch)), "\n")

paths = ["L", "R"]
for a in paths:
    print(bold(a), "\t", tour(subt(tree, a)), "\t", bold(tour(subt(branch, a))))
    for b in paths:
        path = a + b
        print(bold(path), "\t", tour(subt(tree, path)), "\t", bold(tour(subt(branch, path))))
