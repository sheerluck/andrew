def split_world(arr, max):
    if len(arr) < 3:   return [], [], []
    indices = [i for i, x in enumerate(arr) if x == max]
    if [0] == indices: return [], arr[: : -1], []  # leftmost
    left, right = arr[: indices[0] + 1], arr[: indices[-1] - 1: -1]
    chunks, ran = [], list(range(len(indices) - 1))
    for k in ran: chunks.append(arr[indices[k]: indices[k + 1] + 1])
    return left, right, chunks


def puddles(arr):
    indx, length = 0, len(arr)
    if length < 3: return 0
    while (arr[indx + 1] >= arr[indx]):
        indx += 1
        if length == indx + 1: return 0
    left = indx
    while (arr[indx + 1] <  arr[left]): indx += 1
    right = indx + 1
    head, tail = arr[left: right + 1], arr[right:]
    head[-1] = head[0]
    return inner(head) + puddles(tail)


def inner(arr):
    return 0 if len(arr) < 3 else sum([arr[0] - x for x in arr])


world = [2, 5, 1, 3, 1, 2, 1, 7, 6, 3, 1, 3]
left, right, chunks = split_world(world, max(world))
inner_puddles = []
for chunk in chunks: inner_puddles.append(inner(chunk))
print(world, "\nwater=", puddles(left) + puddles(right) + sum(inner_puddles))
