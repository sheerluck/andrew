# -*- coding: utf-8 -*-

namesmap, names_to_show = {}, []

def color( code, txt ): return "\033[1;%sm%s\033[1;m" % (code, txt)
def blue( txt  ): return color( 34, txt )
def green( txt ): return color( 32, txt )
def white( txt ): return color( 37, txt )

prev = None
import fileinput
for line in fileinput.input():
    line    = line[:-1]
    chunk   = line.replace(")", "(")
    parts   = chunk.split("(")
    if 3 != len(parts): continue
    time, of, name = parts
    if " emerge " not in time: continue
    if " of "     not in of:   continue
    if "to "      not in name: continue
    stamp   = time.split(":")[0]
    name    = name.split("to ")[0].strip()
    name    = "%s\t(%s)" % ( green(name), blue(of) )
    isstart = None
    if "::: completed emerge" in time:
        isstart = False
    elif ">>> emerge" in time:
        isstart = True
    else:
        continue

    if isstart == prev and isstart == True: continue

    if name in namesmap.keys():
        if not isstart: continue
        pair = namesmap[ name ]
        pair["start"]    = stamp
        namesmap[ name ] = pair
    else:
        if isstart: continue
        namesmap[ name ] = { "stop" : stamp }
        names_to_show.append( name )

    prev = isstart

import datetime, time
for name in names_to_show:
    pair  = namesmap[name]
    if "start" not in pair.keys(): continue
    delta = int(pair["stop"]) - int(pair["start"])
    start = time.strftime("%d %b %Y %H:%M:%S", time.localtime(int(pair["start"])))
    print( "%s %s\t%s" % ( start, name, white( str( datetime.timedelta(seconds=delta) ).rjust(10) )) )

