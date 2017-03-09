
import util._

("Hell, 2017").toCharArray.map(_.toString).flatMap {o => Try(o.toInt).toOption}  // Array(2, 0, 1, 7)

"expression" intersect "implicitly converted"  // epreion

"implicitly converted" intersect "expression"  // iponere

"implicitly converted".distinct                // implcty onverd

import math._ 

val r = BigInt.probablePrime(100, Random)      // 852844104814964237775339325523

r.toString(36)                                 // 2aon8rqll6x8ibdux7wz

"qwerty".appLYOrElse(14, (o:Int) => o)         // AnyVal = 14

// http://www.scala-lang.org/api/current/scala/collection/immutable/StringOps.html

("implicit" combinations 2) mkString "~"       // ii~im~ip~il~ic~it~mp~ml~mc~mt~pl~pc~pt~lc~lt~ct

"QwErTy" count {_.isUpper}                     // 3

"indecipherability" patch (12, "le vani", 3)   // indecipherable vanity

BigInt(2).pow(64)                              // 18446744073709551616



