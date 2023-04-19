import os
import click
import glob


def init(full: str) -> list[str]:
    repetitive  = [fn[13:] for fn in os.listdir(full)]
    unique = set(repetitive)
    return sorted(unique)


def process(items: list[str], fn: str) -> None:
    for item in items:
        if fn == item:
            return True
    return False


@click.command()
@click.option('--old', default="/tmp/January", help="The days were endless we were crazy we were young")
@click.option('--this', default="2023-04-*",   help="What we've got here is failure to communicate")
def main(old: str, this: str) -> int:
    plain = []
    for fn in glob.glob(this):
        if os.path.isfile(fn):
            if fn.endswith("part"):
                continue
            fn = fn[20:]
            plain.append(fn)
        elif os.path.isdir(fn):
            for fn in os.listdir(fn):
                fn = fn[9:]
                plain.append(fn)

    items = init(old)
    unique = set(plain)
    for fn in sorted(unique):
        if process(items, fn):
            print(f"\033[38;2;211;111;88m{fn}\033[0m")
        else:
            print(f"\033[38;2;119;119;255m{fn}\033[0m")
        
        
    return 0


if __name__ == "__main__":
    exit(main())
