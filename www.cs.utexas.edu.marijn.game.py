import re

print("""
from sage.sat.solvers import Kissat
s = Kissat()""")

fn = "/tmp/boo"
with open(fn, "r") as f:
    for line in f:
        s = re.findall(r'\((.{1,2})\)', line)
        if s:
            t = tuple([int(x) for x in s])
            print(f"s.add_clause({t})")

print("s()")
