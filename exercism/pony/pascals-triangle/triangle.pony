use "collections"
use "itertools"
primitive PascalsTriangle
  fun zip(a: Array[USize], b: Array[USize]): Iter[(USize val, USize val)] =>
    Iter[USize](a.values()).zip[USize](b.values())

  fun rows(c: USize): Array[Array[USize]] =>
    if 1 == c then
      return [[1]]
    elseif c > 1 then
      let prev = rows(c - 1)
      try
        let last = prev(prev.size() - 1)?
        let shL: Array[USize] = last.clone().>unshift(0)
        let shR: Array[USize] = last.clone().>push(0)
        let result = Array[USize]
        for p in zip(shL, shR) do
          result.push(p._1 + p._2)
        end
        prev.push(result)
        return prev
      end
    end
    Array[Array[USize]]
