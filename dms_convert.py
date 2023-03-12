import re

def dms2dec(dms_str):
    '''Return decimal representation of DMS

    > n, e = """ 17°11'77.70"N """,  """  2°20'35.09"E """
    > s, w = """ 17°11'77.70"S """,  """  2°20'35.09"W """
    > dms2dec(n), dms2dec(e)   ->     (17.204917,  2.343081)
    > dms2dec(s), dms2dec(w)   ->    (-17.204917, -2.343081)
    > dms2dec(""" 3°3'E, 6°6'E, 9°9'E """)
    [3.05, 6.1, 9.15]
    '''

    if "," in dms_str:
        return [dms2dec(a) for a in dms_str.split(",")]
    
    dms_str = re.sub(r'\s', '', dms_str.strip())
    
    sign = -1 if re.search('[swSW]', dms_str) else 1
    numbers = [*filter(len, re.split('\D+', dms_str, maxsplit=4))]

    degree = numbers[0]
    minute = numbers[1] if len(numbers) >= 2 else '0'
    second = numbers[2] if len(numbers) >= 3 else '0'
    frac_seconds = numbers[3] if len(numbers) >= 4 else '0'
    
    second += "." + frac_seconds
    r = sign * (int(degree) + float(minute) / 60 + float(second) / 3600)
    return round(r, 6)
