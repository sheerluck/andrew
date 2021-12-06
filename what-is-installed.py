from os import walk
pth, ext = "/var/db/pkg", ".ebuild"
lex, res = len(ext), []
for a, _, u in walk(pth):
    elem = a[lex+5:]
    if "/" in elem:
        res.append(elem)
for pkg in sorted(res, key=lambda x: x.split("/")[1].lower()):
    print(pkg)
