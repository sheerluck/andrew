
import collection.immutable.Seq
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
val g = s.grouped(2).flatMap { _(0) }
g.filter {_>0}.toArray                         // Array(2, 1, 4, 3, 5)

val a = Array(1,2,-1,-2,1,2,3,4,-4,0,-4,9)  
val m = a.groupBy { _>0 }                      // Map(false -> Array(-1, ...), true -> Array(1, ...))
Array(true, false).flatMap { m(_) }            // Array(1, 2, 1, 2, 3, 4, 9, -1, -2, -4, 0, -4)

val a = Array(3.14, 1.59, 2.65, 3.58, 9.79)
val avg = (a.sum, a.size, a.sum/a.size)        // (20.75, 5, 4.15)

val ids  = java.util.TimeZone.getAvailableIDs
val eur  = ids filter {_.startsWith("Europe")} // Array(Europe/Amsterdam, ...
val city = eur map {_.split("/")(1)}           // Array(Amsterdam, Andorra, ...
val u    = city dropWhile {_ < "U"}            // Array(Ulyanovsk, Uzhgorod, Vaduz, Vatican, ...

val scores = Map("Alice" -> 10, "Bob" -> 3, "Cindy" -> 8)

("Alice", 10) == "Alice" -> 10                 // true

val bobsScore = scores.getOrElse("Bob", 0)     // 3

scores - "Alice"                               // Map(Bob -> 3, Cindy -> 8)

for {(k, v) <- scores} yield v -> k            // Map(10 -> Alice, 3 -> Bob, 8 -> Cindy)

import collection.immutable.SortedMap

val balanced = SortedMap("z" -> 9, "a" -> 1)   // SortedMap[String,Int] = Map(a -> 1, z -> 9)
balanced + ("five" -> 5)                       // SortedMap[String,Int] = Map(a -> 1, five -> 5, z -> 9)

import collection.mutable.LinkedHashMap

val order = LinkedHashMap("z" -> 9, "a" -> 1)  // LinkedHashMap[String,Int] = Map(z -> 9, a -> 1) 
order =+ ("five" -> 5)                         // LinkedHashMap[String,Int] = Map(z -> 9, a -> 1, five -> 5)

"how About No".partition(_.isLower)            // ("howbouto", " A N") 
"how About No".span(_.isLower)                 // ("how", " About No")

(List("one", "two") zip List(1, 2)).toMap      // Map(one -> 1, two -> 2)

val env  = System.getenv                       // java.util.Map[String,String] = {PATH=/usr/local/bin ...
import scala.collection.JavaConverters._
SortedMap(mapAsScalaMap(env).toArray:_*)       // Map(ANT_HOME -> /usr/share/ant ...
val p = System.getProperties                   // java.util.Properties = {env.emacs=, ...
val s = propertiesAsScalaMap(p)                // Map(env.emacs -> "",
val f = s filter {_._1.startsWith("java.v")}   // Map(java.vendor.url.bug -> "http ..."

def pp(s: (String, String)) = {
  println(s"${s._1} -> '${s._2}'")
}

SortedMap(f.toArray:_*) foreach pp

// java.vendor -> 'Oracle Corporation'
// java.vendor.url -> 'http://java.oracle.com/'
// java.vendor.url.bug -> 'http://bugreport.sun.com/bugreport/'
// java.version -> '1.8.0_121'
// java.vm.info -> 'mixed mode'
// java.vm.name -> 'Java HotSpot(TM) 64-Bit Server VM'
// java.vm.specification.name -> 'Java Virtual Machine Specification'
// java.vm.specification.vendor -> 'Oracle Corporation'
// java.vm.specification.version -> '1.8'
// java.vm.vendor -> 'Oracle Corporation'
// java.vm.version -> '25.121-b13'

def minmax(v: Array[Int]) = (v.min, v.max)
def lteqgt(v: Array[Int], x: Int) = (v.count(_ < x), v.count(_ == x), v.count(_ > x))

lteqgt(Array(3, 10, 7, -9, -9, 7, 7), -9)      // (0, 2, 5)
lteqgt(Array(3, 10, 7, -9, -9, 7, 7),  7)      // (3, 3, 1) 

LinkedHashMap(("World" zip "Hello":_*))        // Map(W -> H, o -> e, r -> l, l -> l, d -> o)

class Counter {
  private var value = 0          // You must initialize the field
  def increment() { value += 1 } // Methods are public by default
  def current = value            // No () in definition
}

val c = new Counter
c.increment()         //  Use () with mutator
println(c.current)    //  Don’t use () with accessor

class Person(private var _name: String) {
  def name = { print("get"); _name }           // accessor
  def name_=(a: String): Unit = {
    print("set"); _name = a                    // mutator
  }
}

val p = new Person("Bob")
p.name = "Bobby"   // setter
println(p.name)    // getter

// If the field is private, the getter and setter are private.
// If the field is a val, only a getter is generated.
// If you don’t want any getter or setter, declare the field as private[this]

class Message {
  val timeStamp = new java.util.Date // private final field + getter method, no setter.
}

// To summarize, you have four choices for implementing properties:
//   1. var foo: Scala synthesizes a getter and a setter.
//   2. val foo: Scala synthesizes a getter.
//   3. You define methods foo and foo_=  (accessor+mutator).
//   4. You define a method foo.

class Person {
  private var name = ""
  private var age  = 0
  def this(name: String) { this(); this.name = name }  // An auxiliary constructor
  def this(name: String, age: Int) { // Another auxiliary constructor
    this(name) // Calls previous auxiliary constructor
    this.age = age
  }
}

val p1 = new Person             // Primary constructor
val p2 = new Person("Fred")     // First auxiliary constructor
val p3 = new Person("Fred", 42) // Second auxiliary constructor

//  The Primary Constructor

class Person(private val name: String, private val age: Int) {
  println(s"body of The Primary Constructor, n=$name, a=$age")
}

// If a parameter without val or var is used inside at least one method, it becomes a field.

class Person private(val id: Int) // primary constructor is private

class Network(val name: String) { self =>
  class Member(val name: String) {
    def description = this.name + " inside " + self.name
  }
  val m = new Member("Bob")
  println(m.description)
}

val n = new Network("Alice")  // Bob inside Alice

// you can move the Member type from class Network to object Network (no "self=>" syntax)
// or you can use a type projection Network#Member

class Time(val hours: Int, val minutes: Int) {
  private def int (x: Time) = 60*x.hours + x.minutes
  def before(other: Time)   = int(this) < int(other) 
}

val morning = new Time(11, 11)
val evening = new Time(22, 22)
morning before evening                         // Boolean = true

class Employee(val name: String, var salary: Double) {
  def this() { this("John Q. Public", 0.0) }
  override def toString() = s"name=$name, s=$salary"
}
class Employee(val name: String = "John Q. Public",
               var salary: Double = 0.0) {
  override def toString() = s"name=$name, s=$salary"
}

class  Account {
  val id = Account.newUniqueNumber() // from object Account (not in scope!)
  private var balance = 0.0
  def deposit(amount: Double) { balance += amount }
  override def toString() = s"id=$id, b=$balance"
}
object Account { // The companion object (not in scope!)
  private var lastNumber = 0
  private def newUniqueNumber() = { lastNumber += 1; lastNumber }
}

val x = (10 to 13) map { _ => new Account }    // Vector([id=1, b=0.0], ... [id=4, b=0.0])

//  It is easy to confuse Array(100) and new Array(100)
//  "Array(100)"      is "apply(100)", yielding an Array[Int] with a single element, 100
//  "new Array(100)"  is "this(100)",  yielding an Array[Nothing] with 100 null elements.

def uuid = java.util.UUID.randomUUID.toString
class  Meh private (val id: String, val boo: Int)
object Meh {
  def apply(bar: Double) = new Meh(uuid, bar.toInt)
}

Meh(3.14)                                      // [d6727be8-1920-474d-9062-de559096ee4b, 3] 

// Unlike Java or C++, Scala does not have enumerated types. 
object TrafficLightColor extends Enumeration {
  type TrafficLightColor = Value               // for "import TrafficLightColor._"
  val Red, Yellow, Green = Value
  val Blue = Value(0x0000FF)                   // B as in RGB
}
import TrafficLightColor._                     // !!!
for (c <- TrafficLightColor.values.toList)
  yield (c.id + ": " + c)                      // List("0: Red", "1: Yellow", "2: Green", "255: Blue")

def say(color: TrafficLightColor) = {
  if      (Red    == color) "stop"
  else if (Yellow == color) "hurry up"
  else                      "go"
}
say(Green)                                     // "go" 
TrafficLightColor(0)                           // TrafficLightColor.Value = Red
TrafficLightColor.withName("Yellow")           // TrafficLightColor.Value = Yellow
Try(TrafficLightColor(10))                     // Failure(java.util.NoSuchElementException: key not found: 10)

import scala.language.implicitConversions
object Functional {
  class PipedObject[+T] private[Functional] (value:T) {
    def |>[R] (f : T => R) = f(this.value)
  }
  implicit def toPiped[T] (value:T) = new PipedObject[T](value)
}
import Functional._
def boo (a: Int) (b: Int) = {1000 * a + b}
12 |> boo(7)                                   // Int = 7012 
42 |> {_.toDouble} |> {_.toString}             // String = 42.0
(3,4) |> { case(a, b) => a + b}                // Int = 7 
5 |> {7 |> {(a:Int) => (b:Int) => 10*a + b}}   // Int = 75

val x = "abcd".toVector                        // 
// combop may be invoked an N times (even 0)
x.aggregate(0)  ( _ + _.toInt, _ + _ )         // [0,a] -> [97,b] -> [97+98,c] -> [97+98+99, d] -> 97+98+99+100
x.aggregate("0")( _ + _,       _ + _ )         // "0abcd"


// Packages  (":paste -raw" in REPL)
// * can be defined in multiple files
// * do not relate to directory hierarchy
// * can contain a “chain”: package com.ho.imp { ... }
// * Top-of-File Notation without braces
// * can contain classes, objects, and traits, but not the definitions of functions or variables.
//   That’s an unfortunate limitation of the Java virtual machine.
//   But every package can have one package object:
package object people {
  val name = "Andrew"
}

package people {
  class Boo {
    var x = name            // package object people name
    private[people] def bar = "is visible in its own package"
    import scala.collection.mutable._
    // scope of the import statement extends until the end of the enclosing block. 
    import java.util.{HashMap => JavaHashMap}
    import java.util.{HashSet => _} // hides a member instead of renaming it
  }
}

package object random {
  var prev = 0.0
  val p32  = 4294967296L
  def pab  = prev * 1664525 + 1013904223
  def nextDouble(): Double = { prev = pab % p32; prev }
  def nextInt():    Int    = nextDouble.toInt
  def setSeed(seed: Int)   = { prev = seed }
}

package random {
  class Test {
    def test(n: Int) = {
      setSeed(123)
      (0 until n+1) foreach { o => println(nextInt)    }
      (0 to    n)   foreach { o => println(nextDouble) }
    }
  }
}

val base = new random.Test                     // random.Test@1d7d968c
base.test(1)                                   // 121, 186, 1.66E8, 9.4E8

val sys = System.getProperty("user.name")      // sheerluck

class ExString extends String                  // illegal inheritance from final class String
class ExTest   extends random.Test {
  override def test(a: Int) = println(a)
  final def test(a: Double) = {                // so that they cannot be overridden
    super.test(a.toInt)                        // 
  }
  protected[this] val q = 0
  private  [this] val w = 0
}

val der: random.Test = new ExTest              // der: random.Test = ExTest@2fd72332
der.test(1)                                    // 1
der.test(1.1)                                  // error: type mismatch;

// DO NOT ACTUALLY USE IT
val p = if (der.isInstanceOf[ExTest])          // p: ExTest        = ExTest@2fd72332
          der  .asInstanceOf[ExTest] 
        else
          new ExTest

p.test(1.1)                                    // 121, 186, 1.66E8, 9.4E8
// same object, same type:
(der, p)  // (random.Test, ExTest) = (ExTest@2fd72332, ExTest@2fd72332)

// DO NOT ACTUALLY USE IT
base.isInstanceOf[ExTest]                      // false
base.isInstanceOf[random.Test]                 // true
base.getClass == classOf[ExTest]               // false
base.getClass == classOf[random.Test]          // true

der .isInstanceOf[ExTest]                      // true
der .isInstanceOf[random.Test]                 // true
der .getClass == classOf[ExTest]               // true
der .getClass == classOf[random.Test]          // false

// INSTEAD USE PATTERN MATCHING

class Derived(a: Int, b: Int) extends Base(a)  // primary constructor calls the superclass constructor
// In a Scala constructor, you can never call super(param) to call the superclass constructor.

// def can only override another def.
// val can only override another val or a parameterless def.
// var can only override an abstract var 

val x = new Boo(1,2,3) {  // anonymous subclass, type is "Boo{def bar(a: Int): String}"
  def bar(a: Int) = "olo" + "lo" * a
}

def anon(p: Boo{def bar(a: Int): String}) = p.bar(2)

abstract class Person(val name: String) {
  def aa: Int // No method body for abstract method
  val bb: Int // No initializer for abstract field with an abstract getter method
  var cc: String //     Yet another abstract field with abstract getter and setter methods
}

class Employee(val bb: Int) extends Person("\n") { // Subclass has concrete bb property
 var cc = ""                                       //          and concrete cc property
 def aa = name.size
}

val fred = new Person("oh") { //  customize an abstract field by using an anonymous type
  val bb = 123
  var cc = "he"
  def aa = (cc+bb).hashCode   // override keyword not required
}

// The “early definition” syntax lets you initialize val fields of a subclass
// before the superclass is executed. The syntax is so ugly that only a mother could love it.
// You place the val fields in a block after the extends keyword, like this
class Creature {
  val range: Int = 10
  val env: Array[Int] = new Array[Int](range)
}
class Bug extends {
  override val range = 3
} with Creature

// NOTE: At the root of the construction order problem lies a design decision of the Java language
// namely, to allow the invocation of subclass methods in a superclass constructor.
// In C++, an object’s virtual function table pointer is set to the table of the superclass
// when the superclass constructor executes. Afterwards, the pointer is set to the subclass table.
// Therefore, in C++, it is not possible to modify constructor behavior through overriding.
// The Java designers felt that this subtlety was unnecessary, and the Java virtual machine
// does not adjust the virtual function table during construction

val x = new Bug                                //  [3, Array(0, 0, 0)]

// For example, the empty list Nil has type List[Nothing], which is a subtype of List[T] for any T.


//   OBJECT  EQUALITY

// AnyRef:
// * eq     method checks whether two references refer to the same object.
// * equals method calls eq.
// * == operator - for reference types it calls `equals'
// * comparing a fresh object using `eq' will always yield false
// * you should never override the == operator
// * when you define equals, remember to define hashCode as well.
// * for case classes [equals, hashCode] are overridden

class Item(val description: String, val price: Double) {
  final override def equals(other: Any) = {
    val that = other.asInstanceOf[Item]
    if (that == null) false
    else description == that.description && price == that.price
  }
  final override def hashCode = 13 * description.hashCode + 17 * price.hashCode
}

// Вместо целой главы про файлы, я просто https://github.com/pathikrit/better-files

// Вместо сериализации - сравнение "best scala serialization dkomanov"
// Chill (Twitter’s extension for Kryo) 
// ScalaPB (Google Protocol Buffers)
// etc


//  Regular Expressions

val n = """[0-9]+""".r                         // .r -> util.matching.Regex
val w = """\s+[0-9]+\s+""".r                   // Regex = \s+[0-9]+\s+
val m = n.findAllIn("99 bottles, 98 bottles")  // non-empty iterator, 
m.toVector                                     // Vector(99, 98)
n.findFirstIn("99 bottles, 98 bottles")        // Option[String] = Some(99)
// findPrefixOf + replaceFirstIn + replaceAllIn

val n = """([0-9]+) ([a-z]+)""".r              // n(a,b) as "extractor"
val n(num, item) = "99 bottles"                // num: String = 99, item: String = bottles
val c = for (n(x, y) <- m) yield (x, y)        // Iterator[(String, String)] = non-empty iterator
c.toVector                                     // Vector((99,bottles), (98,bottles))

/////////////////////////////////////////////////////////////////////////////////////////

package betterfiles
import better.files._
import java.io.{File => JFile}                 // I personally prefer renaming the Java crap instead

object Main extends App {

  val x = File.home/"git/andrew/selfincrement.py" 
  val y = File.home/"git"/"andrew"/"selfincrement.py"
  val eq = x == y                              // true
  val lines = x.lines                          // Traversable[String] = Buffer("num = 1", "print num", ...)
  val rev = lines.toArray.reverse              // Array[String] = Array("    f.write ... ", ...)
  y.overwrite("").appendLines(rev: _*)         // yes!

}



