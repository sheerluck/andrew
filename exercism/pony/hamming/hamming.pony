use "collections"
use "itertools"
class HammingDistance
  fun zip(a: String, b: String): Iter[(U32 val, U32 val)] =>
    Iter[U32](a.runes()).zip[U32](b.runes())

  fun apply(a: String, b: String): USize ? =>
    if a.size() != b.size() then error end
    var acc: USize = 0
    for pair in zip(a, b) do
      if pair._1 != pair._2 then
        acc = acc + 1
      end
    end
    acc
