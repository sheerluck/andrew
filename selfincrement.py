num = 1
print num
linebreak = '\n'  #  '\r\n'
with open(__file__, mode="rb") as f:
    content    = f.read().split(linebreak)
    def increment(boo): return str(1 + int(boo[1]))
    content[0] = "num = " + increment(content[0].split(' = '))
with open(__file__, mode="wb") as f:
    f.write(linebreak.join(content))
