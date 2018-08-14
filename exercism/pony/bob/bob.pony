class Bob
  fun apply(q: String): String =>
    var s: String ref = q.clone()
    s.strip()
    if 0 == s.size() then
      return "Fine. Be that way!"
    end
    if s.lower() == s.upper() then
      if s.at("?", s.size().isize() - 1) then
        return "Sure."
      end
    else
      if s == s.upper() then
        return "Whoa, chill out!"
      end
    end
    if s.at("?", s.size().isize() - 1) then
      return "Sure."
    end
    "Whatever."
