use "collections"

actor Main
  new create(env: Env) =>
    let p = env.out~print()
    p(intersect("expression", "implicitly converted"))
    p(intersect("implicitly converted", "expression"))

  fun occCounts(s: String): Map[U32, U32] =>
    let m = Map[U32, U32] // {}
    try
      for rune in s.runes() do
        if m.contains(rune) then
          m(rune) = m(rune)? + 1
        else
          m(rune) = 1
        end
      end
    end
    m

  fun intersect(one: String, two: String): String =>
    let occ = occCounts(two)
    let output = recover String(one.size()) end
    for rune in one.runes() do
      try
        let count = occ(rune)?
        if count > 0 then
          output.push_utf32(rune)
          occ(rune) = occ(rune)? - 1
        end
      end
    end
    output
