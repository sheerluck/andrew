# https://github.com/Roguelazer/rdtsc
from time import sleep
from rdtsc import get_cycles
while True:
    asap = [get_cycles() for _ in range(10)]
    for value, prev in zip(asap, [asap[0]] + asap):
        print(value, value - prev)
    print('-' * 15)
    sleep(0.001)  # to prevent 100% CPU
