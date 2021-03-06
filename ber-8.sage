# $ sage ber-8.sage
for a in range(1000): 
    b = bernoulli(a, algorithm='bernmm', num_threads=8) 
    if b: 
        c = n(b, digits=80).str() 
        s = str(b)
        if b > 0:
            c = " " + c
        if "e" in c:
            p1, p2 = c.split("e")
            e = "\033[1;31me\033[1;m"
            c = f"{p1[: 85 - 1 - len(p2) ]}{e}{p2}"
        else:
            c = c[:85]
        if "/" in s: s = s.split("/")[1]
        print(f"B({a:>3}) = {c} # {s}")    
