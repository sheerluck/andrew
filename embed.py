# 1.html -> 2.html with "data:image/{ext};base64,"
from base64 import b64encode as encode
import re

ptrn = re.compile(r'img src="(.+)"')
lines = []
with open("1.html", "r") as html:
    for line in html:
        m = re.search(ptrn, line)
        if m:
            fn = m.group(1)
            ext = fn.split(".")[-1]
            prefix = f"data:image/{ext};base64,"
            with open(fn, 'rb') as f:
                img = f.read()
            res = prefix + encode(img).decode('utf-8')
            lines.append(line.replace(fn, res))
        else:
            lines.append(line)

with open("2.html", "w") as f:
    f.write("".join(lines))
