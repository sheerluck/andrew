class Leap
  fun apply(x: U32): Bool =>
    if 0 == x.mod(400) then
      return true
    end
    if 0 == x.mod(100) then
      return false
    end
    if 0 == x.mod(4) then
      return true
    end
    false
