use "collections"
primitive BeerSong
  fun bob(a: String): String => "bottle" + a + " of beer"
      
  fun numbob(a: USize): String =>
    if 0 == a then
      return "o more " + bob("s")
    end
    if 1 == a then
      return "1 " + bob("")
    end
    a.string() + " " + bob("s")

  fun verse(a: USize): String =>
    let nbob = numbob(a)
    let onw = "on the wall"
    let lin1 = if 0 == a then
                 "N" + nbob + " " + onw + ", n" + nbob + ".\n"
               else
                 nbob + " " + onw + ", " + nbob + ".\n"
               end
    let take = if 0 == a then
                 "Go to the store and buy some more"
               else
                 "Take " + if 1 == a then "it" else "one" end + " down and pass it around"
               end
    let prev = if 0 == a then
                 numbob(99)
               else
                 numbob(a-1)
               end
    let lin2 = if 1 == a then
                 take + ", n" + prev + " " + onw + ".\n"
               else
                 take + ", " + prev + " " + onw + ".\n"
               end
    lin1 + lin2

  fun sing(a: USize, b: USize): String =>
    var x = USize(a)
    let o = recover String((a - b) * verse(0).size()) end
    for i in Range(b, a) do
      o.append(verse(x))
      o.append("\n")
      x = x - 1
    end
    o.append(verse(b))
    o

