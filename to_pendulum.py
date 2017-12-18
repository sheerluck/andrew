from line_profiler import LineProfiler
import pendulum
from inspect import getmembers


def main():
    dt = pendulum.now()
    pairs = [(n, f) for (n, f) in getmembers(dt) if n.startswith("to_")]
    for name, method in pairs:
        print(f"{name:>30}: '{method()}'")


lp = LineProfiler()
lp_wrapper = lp(main)
lp_wrapper()
lp.print_stats()
