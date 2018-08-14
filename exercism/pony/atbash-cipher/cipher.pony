use "collections"
use "itertools"
primitive Atbash
  fun zip(a: String, b: String): Iter[(U32 val, U32 val)] =>
    Iter[U32](a.runes()).zip[U32](b.runes())

  fun map(): Map[U32, U32] =>
    let plain: String = "abcdefghijklmnopqrstuvwxyz0123456789"
    let ciphr: String = "zyxwvutsrqponmlkjihgfedcba0123456789"
    Map[U32, U32].>concat(zip(plain, ciphr))

  fun code(a: String, push32: Bool): String =>
    let m = map()
    var c = USize(1)
    let o = String(50)
    for rune in a.lower().runes() do
      try
        let x = m(rune)?
        o.push_utf32(x)
        if (0 == c.mod(5)) and push32 then
          o.push_utf32(32)
        end
        c = c + 1
      end
    end
    recover o.>strip() end

  fun encode(a: String): String => code(a, true)
  fun decode(a: String): String => code(a, false)
