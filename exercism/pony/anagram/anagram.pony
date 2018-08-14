use "collections"
use "itertools"

class Anagram
  fun toArr(sr: StringRunes): Array[U32] =>
    let result = recover Array[U32] end
    for r in sr do result.push(r) end
    result

  fun zip(a: Array[U32], b: Array[U32]): Iter[(U32 val, U32 val)] =>
    Iter[U32](a.values()).zip[U32](b.values())

  fun apply(src: String, data: Array[String]): Array[String] =>
    let result = recover Array[String] end
    let sorted = Sort[Array[U32], U32](toArr(src.lower().runes()))
    let src_size = src.size()
    for str in data.values() do
      if str.size() != src_size then continue end
      if str        == src      then continue end
      var eq: Bool = true
      let srtd = Sort[Array[U32], U32](toArr(str.runes()))
      for p in zip(sorted, srtd) do
        if p._1 != p._2 then
          eq = false
          break
        end
      end
      if eq == true then result.push(str) end
    end
    result
