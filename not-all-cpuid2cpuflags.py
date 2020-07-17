# search /var/db/pkg/*/* for IUSE > USE
from os import walk
from typing import List, Tuple

pref = "cpu_flags_x86"
lref = len(pref) + 1


def get(full: str) -> list:
    with open(full, 'r') as f:
        line = f.readline()
    flags = [x.strip() for x in line.split(" ")
             if x.startswith(pref)]
    return sorted(flags)


path = "/var/db/pkg"
lath = len(path) + 1
fltr = ["USE", "IUSE"]
result: List[Tuple] = []
for a, _, u in walk(path):
    both = [x for x in u if x in fltr]
    if both:
        fi, fu = sorted([f"{a}/{x}" for x in both])
        ci, cu = get(fi), get(fu)
        li, lu = len(ci), len(cu)
        if li > lu:
            flags = [x[lref:] for x in ci if x not in cu]
            result.append((a[lath:],           # media-sound/mpg123-1.25.10-r1
                           ", ".join(flags)))  # "3dnow, 3dnowext, mmx"

for pkg, flags in sorted(result):
    print(f"{pkg}: {flags}")
