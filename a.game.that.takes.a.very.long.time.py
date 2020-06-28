from datetime import datetime, timedelta, MAXYEAR
from signal import signal, SIGINT, default_int_handler
signal(SIGINT, default_int_handler)


def human_filesize(size):
    if size < 10000:
        return "%u bytes" % size
    units = ["KB", "MB", "GB", "TB"]
    size = float(size)
    divisor = 1024
    for unit in units:
        size = size / divisor
        if size < divisor:
            return "%.1f %s" % (size, unit)
    return "%u %s" % (size, unit)


def human_duration(delta):
    if not isinstance(delta, timedelta):
        delta = timedelta(microseconds=delta * 1000)

    # Milliseconds
    text = []
    if 1000 <= delta.microseconds:
        text.append("%u ms" % (delta.microseconds // 1000))

    # Seconds
    minutes, seconds = divmod(delta.seconds, 60)
    hours, minutes = divmod(minutes, 60)
    if seconds:
        text.append("%u sec" % seconds)
    if minutes:
        text.append("%u min" % minutes)
    if hours:
        text.append("%u hours" % hours)

    # Days
    years, days = divmod(delta.days, 365)
    if days:
        text.append("%u days" % days)
    if years:
        text.append("%u years" % years)
    if 3 < len(text):
        text = text[-3:]
    elif not text:
        return "0 ms"
    return " ".join(reversed(text))


hdd = "/dev/sda"
blocksize = 16 * 1024
counter = 0
start = datetime.now().timestamp()
try:
    with open(hdd, 'rb') as f:
        while True:
            buf = f.read(blocksize)
            if not buf:
                break
            for b in buf:
                counter += 1
                if 0 == b:
                    continue
                if 0x20 <= b <= 0x7E:
                    print(chr(b), end="")
                else:
                    print(f'{b:02X}', end="")
except KeyboardInterrupt:
    print("\n\n")
    stop = datetime.now().timestamp()
    dt = stop - start
    hd = human_duration(1000 * dt)
    print(f"{counter:_} bytes for {hd}")
    v = counter / dt
    print(f"so it's {human_filesize(v)} per second")
    tb4 = 4 * 1024**4
    xt = tb4 * dt / counter
    hd = human_duration(1000 * xt)
    print(f"for 4Tb that would be {hd}")
