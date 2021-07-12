"""$ lddtree /usr/lib64/chromium-browser/chrome | python sorted-lddtree.py."""
from __future__ import annotations

import fileinput
import re
from dataclasses import dataclass
from itertools import count
from typing import Match


@dataclass(order=True)
class Sepulka:
    """Sepulka."""

    off: int
    txt: str
    sepulkarium: list[Sepulka]

    def sepuling(self) -> None:
        """sepuling."""
        print(f'{" " * self.off}{self.txt}')
        for sepulka in sorted(self.sepulkarium, key=lambda o: o.txt.lower()):
            sepulka.sepuling()


def sepuling(level: int,
             sequence: list[tuple[int, str]]) -> list[Sepulka]:
    """sepuling."""
    if 1 == len(sequence):
        txt = sequence[0][1]
        return [Sepulka(level, txt, [])]
    soquence = sequence[1:] + [(-1, "")]
    with_num = []
    for sa, sb, c in zip(sequence, soquence, count()):
        num1, txt = sa
        num2, wat = sb
        value = (num1, txt, "tree" if num2 > num1 else "leaf", c)
        with_num.append(value)

    filtered = list(filter(lambda v: v[0] == level, with_num))
    foltered = filtered[1:] + [(-1, "", "", -1)]
    extended = []
    for fa, fb in zip(filtered, foltered):
        _, txt, what, num = fa
        if what == "tree":
            _, _, _, num2 = fb
            if num2 == -1:
                num2 = 9000
            subseq = sequence[num+1:num2]
            sepulkarium = sepuling(level + 4, subseq)
            sepulka = Sepulka(level, txt, sepulkarium)
        else:
            sepulka = Sepulka(level, txt, [])
        extended.append(sepulka)

    return extended


def main() -> int:
    """First line should be in imperative mood; try rephrasing."""
    lines = []
    sepulka = Sepulka(0, "first line", [])
    for line in fileinput.input():
        f = line[0]
        if f != " ":
            sepulka.txt = line[:-1]
            continue
        m = re.search('[^ ]', line)
        # make mypy happy
        assert isinstance(m, Match)
        o = m.regs[0][0]
        lines.append((o, line[o:-1]))

    sepulka.sepulkarium = sepuling(4, lines)
    sepulka.sepuling()
    return 0


if __name__ == "__main__":
    exit(main())
