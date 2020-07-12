"""

$ qlist -IRv | sed -e "s/::gentoo//" | xargs -n1 qsize
acct-group/input: 0 files, 0 non-files, 0
acct-group/kvm: 0 files, 0 non-files, 0
acct-group/man: 0 files, 0 non-files, 0
acct-group/render: 0 files, 0 non-files, 0
acct-group/sshd: 0 files, 0 non-files, 0
acct-user/man: 0 files, 0 non-files, 0
acct-user/sshd: 1 files, 2 non-files, 0


$ qlist -IRv | sed -e "s/::.\+//" | xargs -n1 qsize | python sort-by-size.py
sci-mathematics/pari-data:                  256.5M
sys-devel/gcc:                              209.9M
dev-texlive/texlive-fontsrecommended:       149.0M
dev-libs/boost:                             133.2M
dev-python/sympy:                           103.6M
dev-lang/python:                             96.6M
app-text/texlive-core:                       80.5M
sci-libs/scipy:                              69.7M
dev-libs/mathjax:                            64.4M
dev-python/pandas:                           63.4M
sys-libs/glibc:                              60.7M
dev-lang/R:                                  57.5M
"""

import fileinput
data = []
for line in fileinput.input():
    ch = line.split(",")
    name = ch[0].split(":")[0]
    raw_size = ch[-1].strip()
    if "K" in raw_size:
        size = float(raw_size[:-1]) * 1024
    elif "M" in raw_size:
        size = float(raw_size[:-1]) * 1024 * 1024
    else:
        size = float(raw_size)
    data.append((name, raw_size, size))

for name, raw_size, size in sorted(data, key=lambda x: x[-1], reverse=True):
    name += ":"
    print(f"{name:<40}{raw_size:>10}")
    if size < 10 * 1024 * 1024:
        break
