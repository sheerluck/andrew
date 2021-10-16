"""."""
import numpy as np
from sys import argv
from time import perf_counter
from psutil import virtual_memory
from parallel_sort import numpy_parallel_sort


def main() -> int:
    """First line should be in imperative mood; try rephrasing."""

    np.set_printoptions(edgeitems=10, linewidth=180)

    size2_3 = int(0.666 * virtual_memory().total)
    intsize = size2_3 // 8
    half = intsize // 2   # Experience is the son of sore mistakes

    rng = np.random.default_rng(5988931115977)
    rints = rng.integers(low=0, high=2**64, size=half, dtype=np.uint64)
    print(f"first 5 {rints[:5]}")
    print(f"last  5 {rints[-5:]}")

    t1 = perf_counter()
    if 2 == len(argv):  # just usual sort
        print("numpy_sort")
        rints.sort()
    else:
        print("numpy_parallel_sort")
        numpy_parallel_sort(rints)
    t2 = perf_counter()
    print(f"first 5 {rints[:5]}")
    print(f"last  5 {rints[-5:]}")
    print(f"for {t2 - t1} sec")

    return 0


if __name__ == "__main__":
    exit(main())
