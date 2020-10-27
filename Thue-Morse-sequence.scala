object Main extends App {
  LazyList.from(0) foreach (u => print(java.lang.Long.bitCount(u) % 2))
}
