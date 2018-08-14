use "collections"
use "itertools"
class ToRNA
  fun apply(s: String): String ? =>
    let a = [as (U32, U32): (67, 71); (71, 67); (84, 65); (65, 85)]
    let m = Map[U32, U32].>concat(a.values())
    let o = recover String(50) end
    for rune in s.runes() do
      let x: U32 = m(rune)?
      o.push_utf32(x)
    end
    o
