import json


class TupleEncoder(json.JSONEncoder):
    def encode(self, o):
        def wrap(item):
            if type(item) is tuple:
                return {'_': 1, '*': item}
            if type(item) is dict:
                return {k: wrap(v) for (k, v) in item.items()}
            if type(item) is list:
                return [wrap(i) for i in item]
            return item
        return super(TupleEncoder, self).encode(wrap(o))


def hook(o):
    if '_' in o and '*' in o and 1 == o['_']:
        return tuple(o['*'])
    return o


from hypothesis import given, strategies as st
from hypothesis import settings as Settings, Verbosity


with Settings(verbosity=Verbosity.verbose):
    @given(items=st.lists(st.tuples(st.integers(), st.text(), st.floats())))
    def test_en_hypo_list(items):
        en = TupleEncoder()
        js = en.encode(items)
        val = json.loads(js, object_hook=hook)
        for a, b in zip(val, items):
            if type(a) is tuple:
                assert type(b) is tuple
            for x, y in zip(a, b):
                from math import isnan
                if type(x) is float and isnan(x):
                    assert isnan(y)
                else:
                    assert x == y
