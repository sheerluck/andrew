from os import walk
pth, ext = "/var/db/pkg", ".ebuild"
lex, res = len(ext), []
for _, _, u in walk(pth): res += [x[:-lex] for x in u if x.endswith(ext)]
for  pkg  in sorted(res): print(pkg)
