import os
import click
import contextlib
import pathlib
from typing import Tuple
from PIL import Image
from colorama import Fore, init as colorama_init


def deslash(s: str) -> str:
    return s.replace("//", "/")


def both(base: str, u: str, u11: str, fn: str) -> Tuple[str, str]:
    if u11.startswith(u):
        info = u11.removeprefix(u)
        bg = f"{base}/{info}/{fn}"
        fg = f"{u11}/{fn}"
    else:
        msg = f"'{u11}' should start with '{u}'"
        raise ValueError(msg)
    return deslash(bg), deslash(fg)


def check_errors(bi: Image, fi: Image) -> None:
    if bi.format != fi.format:
        msg = f"'{bi.format=}' but '{fi.format=}'"
        raise ValueError(msg)

    if bi.size != fi.size:
        msg = f"'{bi.size=}' but '{fi.size=}'"
        raise ValueError(msg)

    if bi.mode != fi.mode:
        msg = f"'{bi.mode=}' but '{fi.mode=}'"
        raise ValueError(msg)


def composite(bg: str, fg: str) -> None:
    bi = Image.open(bg)
    fi = Image.open(fg)
    check_errors(bi, fi)
    print(Fore.CYAN + fg, end=" ")
    print(f"{fi.format}, {fi.mode}, {fi.size}")
    Image.alpha_composite(bi, fi).save(bg)


def process(prod: str, updates: str, upath: str, files: list) -> None:
    for fn in sorted(files):
        if fn.endswith(".png"):
            bg, fg = both(prod, updates, upath, fn)
            if pathlib.Path(bg).is_file():
                composite(bg, fg)
            else:
                print(Fore.RED + f"no base {bg} for update {fg}, skipping")


@click.command()
@click.option('--test',    default="False",                         help='try --test=True')
@click.option('--prod',    default="/media/heights/public/256",     help='path to base')
@click.option('--updates', default="/media/heights/public/updates", help='path to updates')
def main(test: str,
         prod: str,
         updates: str) -> int:
    """First line should be in imperative mood; try rephrasing."""

    if test == "True":
        with contextlib.suppress(ValueError):
            bg, fg = both("/mnt/256/", "a", "b", "100.png")
        bg, fg = both("/mnt/256/", "/mnt/updates/",
                                   "/mnt/updates/8/45/", "100.png")
        print(f"{bg=}")
        print(f"{fg=}")
        return 0

    colorama_init(autoreset=True)
    for a, _, u in os.walk(updates):
        if u:
            process(prod, updates, a, u)

    return 0


if __name__ == "__main__":
    exit(main())
