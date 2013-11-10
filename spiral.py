primes = [1,2,   3,   5,   7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97,
          101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199,
          211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293,
          307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397,
          401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
          503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599,
          601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691,
          701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797,
          809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887,
          907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997]


def init(x):
    matrix = []
    for i in range(x):
        line = []
        for j in range(x):
            char = ' ' if i%2 or j%2 else 0
            line.append(char)
        matrix.append(line)
    return matrix

def print_matrix(x,matrix):
    for i in range(x):
        for j in range(x):
            # '{number:0{width}d}'.format(width=3, number=19)
            print(str(matrix[i][j]).rjust(3),end="")
        print(end="\n")

def main(size):

    def toLeft(m,a,b,cnt):
        while True:
            a += 1
            if a == M: return 0,0,0 # end of spiral
            m[b][a] = '-'
            a += 1
            cnt += 1
            m[b][a] = cnt if cnt in primes else '-'
            if not m[b-2][a]: return a,b,cnt

    def toUp(m,a,b,cnt):
        while True:
            b -= 1
            m[b][a] = '|'
            b -= 1
            cnt += 1
            m[b][a] = cnt if cnt in primes else '|'
            if not m[b][a-2]: return a,b,cnt

    def toRight(m,a,b,cnt):
        while True:
            a -= 1
            m[b][a] = '-'
            a -= 1
            cnt += 1
            m[b][a] = cnt if cnt in primes else '-'
            if not m[b+2][a]: return a,b,cnt

    def toDown(m,a,b,cnt):
        while True:
            b += 1
            m[b][a] = '|'
            b += 1
            cnt += 1
            m[b][a] = cnt if cnt in primes else '|'
            if not m[b][a+2]: return a,b,cnt

    def spiral(m):
        # from 1 to N**2, start is in matrix[M//2][M//2]
        a = b = M//2
        counter = m[a][b] = 1
        # first
        a,b,counter = toLeft(m,a,b,counter)
        # group of 4
        for time in range(M//2):
            for step in [toUp,toRight,toDown,toLeft]:
                a,b,counter = step(m,a,b,counter)
            if 0 == counter: return m


    # N = [3,5,7,9,11,13,15,17,.... 31]
    N = size
    M = N + (N-1)  # N + (empty spaces)
    mtrx = init(M)
    mtrx = spiral(mtrx)
    print_matrix(M,mtrx)
    print(end="\n\n")
    # end of main

if __name__ == '__main__':
    #for size in range(2,32):
    #    if size % 2: main(size)
    main(23)


