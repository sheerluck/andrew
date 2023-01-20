"""  https://xkcd.com/287/


python xkcd287.py 1110                 solutions: 1
x: [0, 0, 1, 1, 1, 0]
1 of side salad : 3.35
1 of host wings : 3.55
1 of mozzarella sticks : 4.2


python xkcd287.py 1500                 solutions: 1
x: [3, 1, 0, 0, 0, 1]
3 of mixed fruit : 2.15
1 of french fries : 2.75
1 of samples place : 5.8


python xkcd287.py 1505                 solutions: 2
x: [7, 0, 0, 0, 0, 0]
7 of mixed fruit : 2.15

x: [1, 0, 0, 2, 0, 1]
1 of mixed fruit : 2.15
2 of host wings : 3.55
1 of samples place : 5.8


python xkcd287.py 2000                 solutions: 6
x: [0, 3, 1, 0, 2, 0]
x: [3, 0, 0, 1, 1, 1]
x: [6, 0, 0, 2, 0, 0]
x: [0, 0, 0, 4, 0, 1]
x: [0, 0, 0, 0, 2, 2]
x: [1, 1, 2, 0, 2, 0]


python xkcd287.py 3000                 solutions: 27
x: [9, 0, 0, 3, 0, 0]
x: [1, 1, 2, 0, 3, 1]
...
x: [1, 3, 2, 2, 0, 1]
x: [2, 1, 3, 2, 0, 1]
"""

import sys
from z3 import Solver, Int, Sum, Or, sat

def main() -> int:
    s = Solver()
    num_prices = 6
    price = [215, 275, 335, 355, 420, 580]
    total = int(sys.argv[1])
    products = ["mixed fruit", "french fries", "side salad",
                "host wings", "mozzarella sticks", "samples place"]
    x = [Int("x%i" % i) for i in range(num_prices)]
    for i in range(num_prices):
        s.add(x[i] >= 0, x[i]<=10)
    # constraints
    s.add(total == Sum([x[i] * price[i]
          for i in range(num_prices)]))
    solutions = 0
    while s.check() == sat:
        solutions += 1
        m = s.model()
        xval = [m.eval(x[i])
                for i in range(num_prices)]
        print("x:", xval)
        for i in range(num_prices):
            if xval[i].as_long() > 0:
                print(xval[i], "of", products[i], ":", price[i] / 100.0)
        print()
        s.add(Or([x[i] != xval[i]
              for i in range(num_prices)]))

    print("solutions:", solutions)
    return 0


if __name__ == "__main__":
    exit(main())
