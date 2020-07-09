import hashlib
import io
from collections import defaultdict

from babel import Locale
from babel.localedata import locale_identifiers
from babel.numbers import format_decimal, format_number, format_percent


def md5(s: str):
    b = s.encode('utf-8')
    return hashlib.md5(b).hexdigest()


ids = [x for x in locale_identifiers() if "_" in x]
locales = sorted(ids)

mlines = {}
mids = defaultdict(list)
mlos = {}
storage = io.StringIO()

for lo in locales:
    storage.seek(0)
    storage.truncate()

    n1 = format_decimal(3.1415, locale=lo, decimal_quantization=False)
    n2 = format_number(199_000, locale=lo)
    n3 = format_percent(0.7777, locale=lo)

    print('\t', n1, file=storage)   # 12,345
    print('\t', n2, file=storage)   # 5.679
    print('\t', n3, file=storage)   # 78%

    lines = storage.getvalue()
    key = md5(lines)
    mlines[key] = lines
    mids[key].append(lo)
    mlos[lo] = key

for lo in locales:
    key = mlos[lo]
    lines = mlines.get(key, "")
    if lines:
        ids = mids[key]

        def to(ID: str):
            name = Locale.parse(ID).get_display_name()
            return f"{name} [{ID}]"

        names = [to(x) for x in ids]
        print(", ".join(names))
        print(lines)
        del mlines[key]
    else:
        continue
