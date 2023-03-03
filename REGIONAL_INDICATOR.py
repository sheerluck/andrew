import sys

def emo(txt: str) -> str:

    def yes(ch) -> bool:
        return 65 <= ord(ch) <= 90

    def tr(ch) -> str:
        return chr(127397 + ord(ch)) if yes(ch) else ch

    u = txt.upper()
    s = chr(0x200C)
    f = True
    r = []
    for a, b in zip(u, u[1:]):
        if f:
            r.append(tr(a))
            f = False
        if yes(a) and yes(b):
            r.append(s)
        r.append(tr(b))

    return "".join(r)
      

def main() -> int:
    txt = "Hello, world"

    if len(sys.argv) > 1:
        txt = " ". join(sys.argv[1:])
    TXT = emo(txt)
    print(TXT)
    return 0


if __name__ == "__main__":
    exit(main())
