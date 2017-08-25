import pendulum
from   inspect import getmembers

dt    = pendulum.now()
pairs = [(n, f) for (n, f) in getmembers(dt) if n.startswith("to_")]
for name, method in pairs:
    print(f"{name:>30}: '{method()}'")
