from os import walk
pth, ext = "/var/db/pkg", ".ebuild"
lex, res = len(ext), {}
for a, _, u in walk(pth):
    try:
        key = [x[:-lex] for x in u if x.endswith(ext)][0]
        res[key.lower()] = a[lex+5:]
    except Exception:
        pass
for  pkg  in sorted(res): print(res[pkg])
