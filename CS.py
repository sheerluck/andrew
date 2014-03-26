ru = "A,Б,В,Г,Д,Е,Ё,Ж,  З,И,Й,К,Л, М,Н,О,П,Р,С,Т,У,Ф, Х,Ц, Ч,  Ш,  Щ, Ъ, Ы,Ь, Э, Ю, Я," \
   + "a,б,в,г,д,е,ё,ж,  з,и,й,к,л, м,н,о,п,р,с,т,у,ф, х,ц, ч,  ш,  щ, ъ, ы,ь, э, ю, я"
en = "A,6,B,r,g,E,E,}l{,3,U,U,K,Jl,M,H,O,n,P,C,T,Y,qp,X,LL,4,LLl,LLL,`b,bl,b,-),l0,9l," \
     "a,6,B,r,g,e,e,}l{,3,u,u,k,Jl,m,H,o,n,p,c,t,y,qp,x,LL,4,LLl,LLL,`b,bl,b,-),l0,9l"
strip_split = lambda lg: [x.strip() for x in lg.split(',')]
ru, en = map(strip_split, [ru, en])

mapa, out, full = {}, [], "CS.txt"
for i, char in enumerate(ru): mapa[char] = en[i]
with open(full, 'rb') as f:
    mbytes = f.read()
text = mbytes.decode(encoding='UTF-8')
for char in text:
    out.append(mapa.get(char, char))
with open(full+'.NEW', 'wb') as f:
    text = ''.join(out)
    mbytes = text.encode(encoding='UTF-8')
    text = f.write(mbytes)
