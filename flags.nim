import strutils, strformat, unicode

func flag(countrycode: string): string =
  let offset = 127397
  let ascii = "abcdefghijklmnopqrstuvwxyz0123456789"
  var code: seq[char] = @[]
  for c in countrycode:
    code.add(c.to_lower_ascii)
  var points: seq[Rune] = @[]
  for c in code:
    points.add(Rune(ord(c.to_upper_ascii) + offset))
  $points

proc main() =
  let codes = ["AF", "AX", "AL", "DZ", "AS", "AD", "AO", "AI",
               "AQ", "AG", "AR", "AM", "AW", "AU", "AT", "AZ",
               "BS", "BH", "BD", "BB", "BY", "BE", "BZ", "BJ",
               "BM", "BT", "BO", "BA", "BW", "BV", "BR", "IO",
               "BN", "BG", "BF", "BI",
               "KH", "CM", "CA", "CV", "KY", "CF", "TD", "CL",
               "CN", "CX", "CC", "CO", "KM", "CG", "CD", "CK",
               "CR", "CI", "HR", "CU", "CY", "CZ",
               "DK", "DJ", "DM", "DO",
               "EC", "EG", "SV", "GQ", "ER", "EE", "ET",
               "FK", "FO", "FJ", "FI", "FR", "GF", "PF", "TF",
               "GA", "GM", "GE", "DE", "GH", "GI", "GR", "GL",
               "GD", "GP", "GU", "GT", "GG", "GN", "GW", "GY",
               "HT", "HM", "VA", "HN", "HK", "HU",
               "IS", "IN", "ID", "IR", "IQ", "IE", "IM", "IL",
               "IT", "JM", "JP", "JE", "JO",
               "KZ", "KE", "KI", "KP", "KR", "KW", "KG",
               "LA", "LV", "LB", "LS", "LR", "LY", "LI", "LT", "LU",
               "MO", "MK", "MG", "MW", "MY", "MV", "ML", "MT",
               "MH", "MQ", "MR", "MU", "YT", "MX", "FM", "MD",
               "MC", "MN", "ME", "MS", "MA", "MZ", "MM",
               "NA", "NR", "NP", "NL", "AN", "NC", "NZ", "NI",
               "NE", "NG", "NU", "NF", "MP", "NO", "OM",
               "PK", "PW", "PS", "PA", "PG", "PY", "PE", "PH",
               "PN", "PL", "PT", "PR", "QA",
               "RE", "RO", "RU", "RW", "BL", "SH", "KN", "LC",
               "MF", "PM", "VC", "WS",
               "SM", "ST", "SA", "SN", "RS", "SC", "SL", "SG",
               "SK", "SI", "SB", "SO", "ZA", "GS", "ES", "LK",
               "SD", "SR", "SJ", "SZ", "SE", "CH", "SY",
               "TW", "TJ", "TZ", "TH", "TL", "TG", "TK", "TO",
               "TT", "TN", "TR", "TM", "TC", "TV", "UG", "UA",
               "AE", "GB", "US", "UM", "UY", "UZ", "VU", "VE",
               "VN", "VG", "VI", "WF", "EH", "YE", "ZM", "ZW"]
  for code in codes:
    let f = flag(code)
    echo &"{code}: {f}"

main()
