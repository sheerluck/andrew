from collections import defaultdict

mod = 10**9 + 7

memo = {}

def subproc(txt: str, t0: int, collected) -> int:

    trio = txt, collected[0]
    answer = memo.get(trio, -1)
    if answer > 0:
        return answer
    count = defaultdict(int)
    printed = False
    if int(txt) <= int(collected[0]):
        #print(f"{txt},{','.join(collected)}")
        printed = True
        count[0] = 1

    lxt = len(txt)
    inloop = False
    for a in range(1, lxt):
        inloop = True
        h, t = txt[:-a], txt[lxt-a:]
        if t.startswith("0"):
            continue
        if int(t) > t0:
            break

        subcol = [t] + collected
        poi = subproc(h, int(t), collected=subcol)
        memo[h, t] = poi
        count[a] = poi

    if not inloop:
        if int(txt) > int(collected[0]):
            return 0
        if printed:
            return sum(count.values()) % mod
        #print(f"{txt},{','.join(collected)}")
        return 1
    return sum(count.values()) % mod


txt = "314159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111"
lxt = len(txt)
acc = 0
for a in range(1, lxt):
    h, t = txt[:-a], txt[lxt-a:]
    if t.startswith("0"):
        continue
    poi = subproc(h, int(t), collected=[t])
    if poi > 0:
        acc = (acc + poi) % mod
        print(f"{poi:>10}, {t}")
print()
print(f"{acc=}")
