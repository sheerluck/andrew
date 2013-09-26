from collections import namedtuple
from math import sin, pi
from fn import _
Tree = namedtuple("Tree", ['f', 'L', 'R'])
leaf = lambda x: Tree(f=lambda a, b: x, L=None, R=None)
visit = lambda t: t.f(visit(t.L), visit(t.R)) if t else None

tree = Tree(f=_ * _, L=leaf(3), R=leaf(5))
print tree, "\n", visit(tree), "\n"

tree = Tree(f=_ * _,
            L=Tree(f=_ + _,
                   L=leaf(11.1),
                   R=leaf(7)
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
print tree, "\n", visit(tree), "\n"
