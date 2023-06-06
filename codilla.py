# input -- some random binary file,
#          zzz.mp3 or 天狗.webm
# outpt -- /tmp/pakla

import os
import sys
import galois
import struct
from math import ceil
from rich import print
from tqdm import tqdm
from datetime import datetime
from typing import Any, Final, TypeAlias, Generator


Nibble: TypeAlias = bytes
Anyerator: TypeAlias = Generator[Any, None, None]
Byterator: TypeAlias = Generator[bytes, None, None]
Liberator: TypeAlias = Generator[list[bytes], None, None]


nibble: Final[int] = 4
nistep: Final[int] = 1980


def flatten(it: list[Any] | tuple[Any, ...]) -> Anyerator:
    for x in it:
        if isinstance(x, (list, tuple)):
            yield from flatten(x)
        else:
            yield x


def int4bytes(x: int) -> bytes:
    if 0 == x:
        return b"\x00\x00\x00\x00"
    return x.to_bytes(4, byteorder="little", signed=False)


def content(full: str) -> bytes:
    with open(full, "rb") as f:
        return f.read()


def blice(bb: bytes) -> Byterator:
    for i in range(0, len(bb), 8):
        block = bb[i : i + 8]
        while len(block) < 8:
            block = b"".join([block, b"\x00"])
        yield block


def one(it: list[Nibble | int]) -> Byterator:
    for i in range(0, len(it), 2):
        a, b = it[i : i + 2]
        # make mypy happy
        assert isinstance(a, bytes)
        assert isinstance(b, bytes)
        a, b = a[0], b[0]
        n = (a << 4) | b
        yield bytes([n])


def two(it: bytes) -> Liberator:
    for i8 in it:
        a = bytes([i8 >> 4])
        b = bytes([i8 & 15])
        yield [a, b]


def get_in(it: list[Nibble | int], step: int) -> list[Nibble | int]:
    lenit = len(it)
    in_ = [0] * lenit
    res: list[Nibble | int] = [0] * lenit
    pos = step
    while in_:
        elem = in_.pop()
        while it[pos % lenit] == 0:
            pos += 1
        res[len(in_)] = it[pos % lenit]
        it[pos % lenit] = elem
        pos = (pos + step) % lenit
    return res


def get_out(it: list[Nibble], step: int) -> list[Nibble | int]:
    it = it[:]
    lenit = len(it)
    out: list[Nibble | int] = [0] * lenit
    pos = step
    while it:
        elem = it.pop()
        while out[pos % lenit] != 0:
            pos += 1
        out[pos % lenit] = elem
        pos = (pos + step) % lenit
    return out


def to_32(bch: galois.BCH, b8: bytes) -> list[Nibble]:
    s8 = "-".join([format(byte, "08b") for byte in b8])
    l8 = [int(bit) for bit in s8 if bit in ["0", "1"]]
    en = bch.encode(l8)
    binary_list = en.tolist() + [0]
    binary_bytes = []
    for j in range(0, len(binary_list), nibble):
        ch = binary_list[j : j + nibble]
        binary_str = "".join(str(bit) for bit in ch)
        binary_int = int(binary_str, 2)
        binary_byte = binary_int.to_bytes(1, byteorder="big")
        binary_bytes.append(binary_byte)
    return binary_bytes


def from_32(bch: galois.BCH, x32: list[Nibble | int]) -> list[bytes]:
    q127: list[int] = []
    for a in x32:
        assert isinstance(a, bytes)
        s8 = bin(a[0])[2:].rjust(8, "0")  # very ... nice
        s4 = s8[4:]
        l4 = [int(x) for x in s4]
        q127.extend(l4)
    de = bch.decode(q127[:127])
    binary_list = de.tolist()
    binary_bytes = []
    for j in range(0, len(binary_list), 8):
        ch = binary_list[j : j + 8]
        binary_str = "".join(str(bit) for bit in ch)
        binary_int = int(binary_str, 2)
        binary_byte = binary_int.to_bytes(1, byteorder="big")
        binary_bytes.append(binary_byte)
    return binary_bytes


def encode(bch: galois.BCH, fn: str) -> None:
    # 8 bytes - len of fn
    # 8 bytes - len of file
    # basename
    # content
    bsn = os.path.basename(fn)
    print(f"ℹ️ name is [red]{bsn}[/red]")
    bfn = bsn.encode("utf-8")
    leb = int4bytes(len(bfn))
    A32 = to_32(bch, b"".join([leb, leb]))
    fsize = os.path.getsize(fn) % 2**32
    print(f"ℹ️ file is [red]{fsize}[/red] bytes long")
    leb = int4bytes(fsize)
    B32 = to_32(bch, b"".join([leb, leb]))
    C32 = [to_32(bch, b8) for b8 in blice(bfn)]
    D32 = []
    with tqdm(total=fsize * 4) as progress:
        for b8 in blice(content(fn)):
            D32.append(to_32(bch, b8))
            progress.update(32)
    to_write = [A32, B32, C32, D32]
    nibs = [nib for nib in flatten(to_write)]
    print(f"✅ we got [red]{len(nibs)}[/red] nibbles")
    this = get_out(nibs, nistep)
    fn = "/tmp/pakla"
    print(f"👍 writing to [red]{fn}[/red]!")
    write(fn, this)


def write(fn: str, this: list[Nibble | int]) -> None:
    with open(fn, "wb") as fdst:
        for b8 in one(this):
            fdst.write(b8)


def read(c: bytes) -> list[Nibble | int]:
    this: list[Nibble | int] = []
    for a, b in two(c):
        this.extend([a, b])
    return this


def unpack(w: list[bytes]) -> int:
    by: bytes = b"".join(w)
    qq = struct.unpack("<I", by)
    assert isinstance(qq[0], int)
    return qq[0]


def unsize(s8: list[bytes]) -> int:
    h, t = s8[:4], s8[4:]
    a, b = unpack(h), unpack(t)
    if a != b:
        print(f"❌ [red]which one, {a} of {b}?[/red]")
        print(f"❌ [cyan]This is so wrong I even can't![/cyan]")
        raise ValueError("size is wrong")
    return a


def extract_name(bch: galois.BCH,
                 n: int,
                 nibs: list[Nibble | int]) -> str:
    items: list[bytes] = []
    while nibs:
        ch = nibs[:32]
        nibs = nibs[32:]
        raw = from_32(bch, ch)
        items.append(b"".join(raw))
    bname = b"".join(items)[:n]
    return bname.decode("utf-8")


def extract_content(bch: galois.BCH,
                    fn: str,
                    n: int,
                    nibs: list[Nibble | int]) -> None:
    print(f"✅ we got [red]{len(nibs)}[/red] nibbles")
    with open(fn, "wb") as f:
        with tqdm(total=len(nibs) / 4) as progress:
            ch = []
            for nib in nibs:
                ch.append(nib)
                if len(ch) < 32:
                    continue
                f.write(b"".join(from_32(bch, ch)))
                progress.update(8)
                ch = []
        f.seek(n)
        f.truncate()


def decode(bch: galois.BCH) -> None:
    fn = "/tmp/pakla"
    print(f"ℹ️ name is [red]{fn}[/red]")
    this = read(content(fn))
    nibs = get_in(this, nistep)
    A32, B32, cd = nibs[:32], nibs[32:64], nibs[64:]
    fnlen = unsize(from_32(bch, A32))
    fsize = unsize(from_32(bch, B32))
    print(f"ℹ️ file is [red]{fsize}[/red] bytes long")
    blcks = ceil(fnlen / 8)
    head, tail = cd[: 32 * blcks], cd[32 * blcks :]
    base = extract_name(bch, fnlen, head)
    suff = datetime.now().strftime("%Y-%m-%d-%H-%M-%S-%f")
    fn = f"{base}.{suff}"
    print(f"ℹ️ new name is [red]{fn}[/red]")
    extract_content(bch, fn, fsize, tail)


def main() -> int:
    print(f"👉 [red]Creating BCH...[/red]")
    bch: Final[galois.BCH] = galois.BCH(127, 64)
    try:
        fn = sys.argv[1]
        encode(bch, fn)
    except IndexError:
        decode(bch)
    print("All done! ✨ 🍰 ✨")
    return 0


if __name__ == "__main__":
    exit(main())
