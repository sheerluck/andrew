# -*- coding: utf-8 -*-
import attr
from stackoverflow import getsize


@attr.s(slots=True, frozen=True)
class Point3D:
    x, y, z = attr.ib(), attr.ib(), attr.ib()


a, b = (1, 2, 3), Point3D(1, 2, 3)
print(f"size(a)={getsize(a)}, size(a)={getsize(b)}")
