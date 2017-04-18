package floatin3bytes
import scala.collection.mutable.ListBuffer
import java.nio.ByteBuffer

object Main
{
  def color  (c:Int, s:String): String = "\u001b[1;" :: c.toString :: "m" :: s :: "\u001b[0m" :: Nil mkString("")
  def padLeft(n:Int, s:String): String = "0"*(n - s.length) + s
  def flo2int(f:Float): Int = ByteBuffer.wrap(ByteBuffer.allocate(4).putFloat(f).array()).getInt
  def int2flo(i:Int): Float = ByteBuffer.wrap(ByteBuffer.allocate(4).putInt( i ).array()).getFloat

  def repr   (a:Int, blue:Int, green:Int, red:Int): String = {
    val key  = (a & 0x00E00000) >>> 21
    val body = (a & 0x001FFFFF)

    val colors = List(32, 97, 31)
    val binstr = padLeft(green, key.toBinaryString) :: "." :: padLeft(red, body.toBinaryString) :: Nil
    var lb = new ListBuffer[String]()
    for ((c,s) <- (colors zip binstr))   lb += color(c, s)
    return lb.toList.mkString("")
  }

  def hex(a:Int, size:Int): String = {
    val h = padLeft(size, a.toHexString.toUpperCase)
    val x = h.grouped(2).toList.mkString(".")
    return "[" :: x :: "]" :: Nil mkString("")
  }

  def hex3(a:Int  ): String = hex(a,          6)  // "[xx.xx.xx]"
  def hex4(f:Float): String = hex(flo2int(f), 8)  // "[xx.xx.xx.xx]"

  def unpack0321(a:Int): Float =
  {
    val key  = (a & 0x00E00000) >>> 21
    val body = (a  <<  2) & 0x007FFFFC
    val exp  = key + 126
    val head = (exp << 23) & 0x7F800000
    return int2flo(head | body)
  }

  def printFloat(cnt24:Int)
  {
    val float  = unpack0321(cnt24)
    val vrepr  = repr(cnt24,0,3,21)
    val vhex3  = hex3(cnt24)
    val vhex4  = hex4(float)
    val result = "\t%s\t%s\t%s\t%1.22f\n".format(vrepr, vhex3, vhex4, float)
    print(result)
  }

  def main(args: Array[String])
  {
    val max24 = 0x00FFFFFF
    var cnt24 = 0x0
    while (cnt24 <= max24)
    {
      printFloat(cnt24)
      cnt24 += 1
    }
  }
}
