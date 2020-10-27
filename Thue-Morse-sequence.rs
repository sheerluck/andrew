fn main() {
    (0u64..).for_each(|n| print!("{}", n.count_ones() % 2))
}
