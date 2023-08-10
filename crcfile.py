import os
import sys
import itertools
from fastcrc import crc16, crc32, crc64
import xxhash
import hashlib
import tlsh
from blake3 import blake3
from portage.util.whirlpool import CWhirlpool
from Crypto.Hash import MD4
from Crypto.Hash import KangarooTwelve as K12
import kerukuro_digestpp


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


def one_file(fn) -> int:
    print(f"\n\n\033[38;2;119;119;255m{fn}\033[0m")
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
    print(f"{name:<17}: {int2str(i)[-64:]}")

    for (name, f) in sorted(itertools.chain.from_iterable(crc)):
        if "iso_hdlc" in name or "xz" in name:
            print(f"{name:<17}: {hex(f(data))[2:]}")

    def sort(p):
        if "xxh3_" in p[0]:
            key = p[0] + "*" * 100 
        else:
            key = str(p[1](data))
        return len(key)

    alg = xxhash.algorithms_available
    fun = [(f"xxhash.{f}", eval(f"xxhash.{f}_hexdigest")) for f in alg]
    for name, f in sorted(fun, key=sort):
        if "xxh128" in name:
            continue
        print(f"{name:<17}: {f(data)}")

    h = MD4.new()
    name, f = "md4", lambda x: (h.update(x), h.hexdigest())[-1]
    print(f"{name:<17}: {f(data)}")

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
        print(f"{name:<17}: {val}")
        if "md5" in name:
            h = hashlib.new("ripemd160")
            name, f = "hashlib.ripemd160", lambda x: (h.update(x), h.hexdigest())[-1]
            print(f"{name:<17}: {f(data)}")

    h = K12.new()
    for name, f in zip(["K12-512", "blake3-512", "Whirlpool", "TLSH"],
                       [lambda x: (h.update(x), h.read(64).hex())[-1],
                        lambda x:  blake3(x).hexdigest(64),
                        lambda x:  CWhirlpool(x).hexdigest(),
                        lambda x:  tlsh.hash(x)]):
        print(f"{name:<17}: {f(data)}")

    exotic = ["echo", "esch", "groestl", "jh", "kupyna", "skein"]
    fun = []
    for f in exotic:
        for q in ["256", "512"]:
            pair = f"{f}{q}", eval(f"kerukuro_digestpp.{f}{q}")
            fun.append(pair)
    for name, f in fun:
        print(f"{name:<17}: {f(fn)}")

    return 0


def main(flist=sys.argv[1:]) -> int:
    for fn in sorted(flist):
        if os.path.isfile(fn):
            one_file(fn)
        if os.path.isdir(fn):
            try:
                lifted = [os.path.join(fn, a) for a in os.listdir(fn)]
                main(lifted)
            except PermissionError:
                pass
    return 0


if __name__ == "__main__":
    exit(main())
