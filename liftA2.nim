import options, re, sequtils, strutils, sugar


type
  Si = seq[int]
  Oi = Option[int]
  Os = Option[seq[int]]
  Fs = (Si) -> Si
  IF = (int) -> Fs


proc cons(x: int): Fs =
  proc boo(xs: Si): Si =
    return @[x] & xs
  return boo


proc liftA2(f: IF, ma: Oi, mb: Os): Os =
  if ma.is_none:
    return none(Si)
  if mb.is_none:
    return none(Si)
  return f(ma.get)(mb.get).some


proc get_all_numbers(): Os =
  stdout.write "Введите что угодно с запятыми: "
  let s = read_line(stdin)
  let q = find_all(s, re"\d+")
  return map(q, parse_int).some


proc get_first_number(): Oi =
  stdout.write "Введите что угодно без запятых: "
  let s = read_line(stdin)
  for ma in find_all(s, re"\d+"):
    return ma.parse_int.some
  return none(int)


proc main: int =
  let xs = get_all_numbers()
  let oh = get_first_number()
  echo liftA2(cons, oh, xs)
  return 0

when is_main_module:
  quit(main())
