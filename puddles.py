def split_world(arr):
    ma = max(arr)
    indices = [i for i, x in enumerate(arr) if x == ma]
    if 1 == len(indices):
        if 0 == indices[0]: return split_world(arr[::-1])
        return arr[: indices[0] + 1], arr[: indices[0] - 1: -1], []
    chunks = []
    for k in range(len(indices) - 1):
        chunk = arr[indices[k]: indices[k + 1] + 1]
        chunks.append(chunk)
    return arr[: indices[0] + 1], arr[: indices[-1] - 1: -1], chunks


def puddles(arr):
    if len(arr) < 3: return 0
    ind = 0
    while (arr[ind + 1] >= arr[ind]):
        ind += 1
        if len(arr) == ind + 1: return 0
    left = ind
    while (arr[ind + 1] < arr[left]): ind += 1
    right = ind + 1
    head, tail = arr[left:right + 1], arr[right:]
    head[-1] = head[0]
    return inner(head) + puddles(tail)


def inner(arr):
    if len(arr) < 3: return 0
    ma = max(arr)
    water = [ma - x for x in arr]
    return sum(water)


def main():
    world = [2,5,1,3,1,2,1,7,6,3,1,3]
    left, right, chunks = split_world(world)
    inner_puddles = []
    for chunk in chunks: inner_puddles.append(inner(chunk))
    print world, "\nwater=", puddles(left) + puddles(right) + sum(inner_puddles)


if __name__ == '__main__':
    main()
