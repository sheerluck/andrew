"""
Use this if you are afraid of https://github.com/sagemath/sage/blob/develop/src/sage/misc/decorators.py
"""

from functools import partial
from functools import update_wrapper as refresh
from functools import wraps as cmon
from inspect   import signature


def decorator(g=None):  # decorator to make a decorator

    class Holder:
        def __init__(self, f, a, kw):
            self.f, self.a, self.kw = f, a, kw
            refresh(self, f)
        def __call__(self):
            return self.f(*self.a, **self.kw)

    def inner(ff, *ff_a, **ff_kw):
        signature(ff).bind(None, *ff_a, **ff_kw)  # validate
        @cmon(ff)
        def decorate(func):
            @cmon(func)
            def call_(*a, **kw):
                thing = Holder(func, a, kw)
                return ff(thing, *ff_a, **ff_kw)
            return call_
        return decorate
    if g:
        return inner(g)
    return lambda f: refresh(partial(inner, f), f)
