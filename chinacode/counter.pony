"""

Бывают разные числа.
Вот по основанию  2:  100101010111110101101
Вот по основанию  8:  234543253452345233253
Вот по основанию 10:  1234567890
Вот по основанию 16:  450bad054
А я придумал числа по основанию 5000:  䉘䗵㷺䁡㹿㚴
А потом числа по основанию 20000: 乂蚌烘鎑諤靆


"""

use "time"
use "collections"

primitive China
  fun u2c(a: U128): String =>
    """ 123456789 -> 昼辵 """
    var x: U128     = a
    let n: U128     = 20_000  // 40_000 - 20_000
    let shift: U32  = 20_000  // china magic
    let bits = recover List[U32] end
    while x >= n do
      let div: U128 = x.div(n)
      let mod: U32  = x.mod(n).u32()
      bits.unshift(shift + mod)
      x = div
    end
    bits.unshift(shift + x.u32())
    let out = recover String(50) end
    for u in (consume bits).values() do
      out.push_utf32(u)
    end
    out

class Printer is TimerNotify
  let _env: Env 
  var _counter: U128
  new iso create(env: Env, initial: U128) =>
    _env = env
    _counter = initial

  fun ref apply(timer: Timer, count: U64): Bool =>
    let str = China.u2c(_counter)
    _env.out.print(str)
    _counter = _counter + 1
    true

actor Main
  new create(env: Env) =>
    let timers = Timers
    let timer = Timer(Printer(env, 12345678901234567890), 0, 5_000_000)
    timers(consume timer)

