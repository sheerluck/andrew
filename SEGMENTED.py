from datetime import datetime
usual = f"{datetime.now():%F %T}"
print(usual)

segmented = [(chr(130032 - 48 + o)
              if 48 <= (o := ord(ch)) <= 57
              else ch) for ch in usual]
print("".join(segmented))
