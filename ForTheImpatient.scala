
import util._

("Hell, 2017").toCharArray.map(_.toString).flatMap {o => Try(o.toInt).toOption}  // Array(2, 0, 1, 7)

"expression" intersect "implicitly converted"  // epreion

"implicitly converted" intersect "expression"  // iponere

"implicitly converted".distinct                // implcty onverd

import math._ 

val r = BigInt.probablePrime(100, Random)      // 852844104814964237775339325523

r.toString(36)                                 // 2aon8rqll6x8ibdux7wz

"qwerty".applyOrElse(14, (o:Int) => o)         // AnyVal = 14

// http://www.scala-lang.org/api/current/scala/collection/immutable/StringOps.html

("implicit" combinations 2) mkString "~"       // ii~im~ip~il~ic~it~mp~ml~mc~mt~pl~pc~pt~lc~lt~ct

"QwErTy" count {_.isUpper}                     // 3

"indecipherability" patch (12, "le vani", 3)   // indecipherable vanity

BigInt(2).pow(64)                              // 18446744073709551616

for {
  i <- 1 to 3
  j <- 1 to 3
  if i != j
} print(s"${10 * i + j} ")                     // 12 13 21 23 31 32

for (c <- "Hello"; i <- 0 to 1) yield (c + i).toChar  // HIeflmlmop
for (i <- 0 to 1; c <- "Hello") yield (c + i).toChar  // Vector(H, e, l, l, o, I, f, m, m, p)

def sum(args: Int*) = args.sum

sum(1 to 100: _*)                              // 5050

(10 to 0 by -1) foreach print                  // "downto": 109876543210 

def countdown = (o:Int) => (o to 0 by -1) foreach println

"Hello".map (_.toLong).product                 // Long = 9415087488 
"Hello".map (_.toInt ).product                 // Int  =  825152896

def xn(x: Int, n: Int): Double = {
  n match {
    case m if m < 0      => 1.0 / xn(x, -n)
    case m if 0 == m     => 1.0
    case m if 0 == m % 2 => {
      val y = xn(x, n / 2)
      y * y 
    }
    case _ => x * xn(x, n - 1)
  }
}

def test_xn = xn(2, _:Int)

(-3 to 4).map {test_xn}                        // Vector(0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0, 16.0)

// Use an Array if the length is fixed, and an ArrayBuffer if the length can vary.

import collection.mutable.ArrayBuffer
val ab = ArrayBuffer[Int]()
ab += (1, 2, 3)                                // ArrayBuffer(1, 2, 3)
ab ++= Array(10, 11, 12)                       // ArrayBuffer(1, 2, 3, 10, 11, 12)
ab.trimStart(2)                                // ArrayBuffer(3, 10, 11, 12)
ab.trimEnd(2)                                  // ArrayBuffer(3, 10) 
ab += (7, 7, 7)                                // ArrayBuffer(3, 10, 7, 7, 7)
ab.insert(3, -9, -9)                           // ArrayBuffer(3, 10, 7, -9, -9, 7, 7)
ab.remove(0, 2)                                // ArrayBuffer(7, -9, -9, 7, 7)
val a = ab.toArray                             // Array(7, -9, -9, 7, 7)
a filter { _ % 3 == 0 } map { -2 * _ }         // Array(18, 18)

// Most of business computations are nothing but computing sums and sorting.

Sorting.quickSort(a)                           // Array(-9, -9, 7, 7, 7) in place!
a.mkString("<", ", ", ">")                     // "<-9, -9, 7, 7, 7>"
a.padTo(7, 3)                                  // Array(-9, -9, 7, 7, 7, 3, 3)  padRight!
Array.fill(7 - a.size)(3) ++ a                 // Array(3, 3, -9, -9, 7, 7, 7)  padLeft!

val matrix = Array.ofDim[Double](3, 4)         // Array( 3 x Array(0.0, 0.0, 0.0, 0.0))
val m3     = Array.ofDim[Int](3, 4, 5)         // Array( 3 x Array( 4 x Array(0, 0, 0, 0, 0)))
val (row, column, wtf) = (2, 3, 4)
m3(row)(column)(wtf) = 42                      // Array(Array(Array(0, ... 0, 42)))

val triangle = new Array[Array[Int]](10)       // Array(10 x null)     WTF?
for (i <- 0 until triangle.length)
  triangle(i) = new Array[Int](i + 1)          // Array(Array(0), Array(0, 0), ... Array(10x0))

val a = Array(1, 2, 3, 4, 5)
val b = a zip a.tail ++ Array(-1)              // Array((1,2), (2,3), (3,4), (4,5), (5,-1))
val s = b.map { case (a, b) => Array(b, a)}    // Array(Array(2, 1), Array(3, 2) ...Array(-1, 5))
val g = s.grouped(2).map { _(0) }.flatten
g.filter {_>0}.toArray                         // Array(2, 1, 4, 3, 5)

val a = Array(1,2,-1,-2,1,2,3,4,-4,0,-4,9)  
val m = a.groupBy { _>0 }                      // Map(false -> Array(-1, ...), true -> Array(1, ...))
Array(true, false).map { m(_) }.flatten        // Array(1, 2, 1, 2, 3, 4, 9, -1, -2, -4, 0, -4)

val a = Array(3.14, 1.59, 2.65, 3.58, 9.79)
val avg = (a.sum, a.size, a.sum/a.size)        // (20.75, 5, 4.15)

val ids  = java.util.TimeZone.getAvailableIDs
val eur  = ids filter {_.startsWith("Europe")} // Array(Europe/Amsterdam, ...
val city = eur map {_.split("/")(1)}           // Array(Amsterdam, Andorra, ...
val u    = city dropWhile {_ < "U"}            // Array(Ulyanovsk, Uzhgorod, Vaduz, Vatican, ...


