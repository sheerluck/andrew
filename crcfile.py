import sys
import itertools
from fastcrc import crc16, crc32, crc64
import xxhash
import hashlib
from blake3 import blake3
from portage.util.whirlpool import CWhirlpool


def radix(val, base):
    res = []
    while True:
        head, tail = divmod(val, base)
        res.append(tail)
        if head < base:
            if head > 0:
                res.append(head)
            return res
        val = head


def int2str(x: int) -> str:
    base = 20_180  # 40_940 - 20_760
    shift = 20_760
    bits = []
    for mod in radix(x, base):
        bits.append(chr(shift + mod))
    return "".join(reversed(bits))


def int2bytes(x: int) -> bytes:
    if 0 == x:
        return b"\x00"
    return x.to_bytes((x.bit_length() + 7) // 8)


def content(full: str) -> bytes:
    with open(full, "rb") as f:
        return f.read()


def main() -> int:
    fn = sys.argv[1]
    data = content(fn)
    crc = []
    for mod in ["crc16", "crc32", "crc64"]:
        alg = eval(f"{mod}.algorithms_available")
        fun = [(f"{mod}.{f}", eval(f"{mod}.{f}")) for f in alg]
        crc.append(fun)

    fun = [f for (n, f) in sorted(itertools.chain.from_iterable(crc))]
    hsh = [str(f(data)) for f in fun]
    line = "".join(hsh)
    i = int(line)
    name = "crc.combined"
    print(f"{name:<25}: {int2str(i)[-70:]}")

    def sort(p):
        if "xxh3_" in p[0]:
            key = p[0] + "*" * 100 
        else:
            key = str(p[1](data))
        return len(key)

    alg = xxhash.algorithms_available
    fun = [(f"xxhash.{f}", eval(f"xxhash.{f}_intdigest")) for f in alg]
    for name, f in sorted(fun, key=sort):
        print(f"{name:<25}: {f(data)}")

    def sort(p):
        name, f = p 
        try:
            key = f(data).hexdigest()
        except Exception:
            suf = int(name[-3:])
            key = "k" * 32 + "*" * (suf // 100)
        if "sha3_" in name:
            key += "*" * 100 
        if "blake" in name:
            key += "*" * 200 
        return len(key)

    alg = hashlib.algorithms_guaranteed
    fun = [(f"hashlib.{f}", eval(f"hashlib.{f}")) for f in alg]
    for name, f in sorted(fun, key=sort):
        try:
            val = f(data).hexdigest()
        except Exception:
            suf = int(name[-3:])
            val = f(data).hexdigest(suf // 8)
        print(f"{name:<25}: {val}")

    name, f = f"blake3.blake3_{8 * 64}", lambda b: blake3(b).hexdigest(64)
    print(f"{name:<25}: {f(data)}")
    name, f = "Whirlpool", lambda x: CWhirlpool(x).hexdigest()
    print(f"{name:<25}: {f(data)}")

    return 0


if __name__ == "__main__":
    exit(main())
