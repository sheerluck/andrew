class RomanNumerals
  fun apply(a: USize): String ? =>
    if a < 0 then error end
    if a > 3000 then error end
    let rnm = [as (String, USize):
               ("M", 1000)
               ("CM", 900)
               ("D",  500)
               ("CD", 400)
               ("C",  100)
               ("XC",  90)
               ("L",   50)
               ("XL",  40)
               ("X",   10)
               ("IX",   9)
               ("V",    5)
               ("IV",   4)
               ("I",    1)]
    var n: USize val = a // bc a is not var
    let o = recover String(10) end
    for p in rnm.values() do
      while n >= p._2 do
        o.append(p._1)
        n = n - p._2
      end
    end
    o
