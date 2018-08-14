
class Squares
  fun square_of_sums(a: USize): USize =>
    var acc: USize = 0
    var i: USize = a
    repeat
      acc = acc + i
      i = i - 1
    until i < 1
    end
    acc * acc
  fun sum_of_squares(a: USize): USize =>
    var acc: USize = 0
    var i: USize = a
    repeat
      acc = acc + (i * i)
      i = i - 1
    until i < 1
    end
    acc

  fun difference(a: USize): USize =>
    square_of_sums(a) - sum_of_squares(a)

