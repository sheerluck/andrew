import string


def init(mass="full"):
    m = {}
    # part 0: [0..9]
    for a in range(10):
        m[a] = str(a)
    if "small" == mass:
        for n, a in enumerate(string.ascii_lowercase, start=10):
            m[n] = a
        return m
    # part 1: [A..Z]
    for n, a in enumerate(string.ascii_uppercase, start=10):
        m[n] = a
    # part 1.5: [a..z]
    for n, a in enumerate(string.ascii_lowercase, start=36):
        m[n] = a
    # part 2: [🐀..🐽]
    for n, a in enumerate(range(62), start=62):
        m[n] = chr(a + 128000)
    # part 3: [🦀..🦭]
    for n, a in enumerate(range(46), start=124):
        m[n] = chr(a + 129408)

    return m


def not_pi() -> "real":
    s = n(pi, digits=19000).str(no_sci=2)
    return RealField(60000)(s)


def frac(m: dict, base: int) -> str:
    r = []
    p = not_pi() - 3
    for a in range(150):
        s = n(p * base).str(no_sci=2)
        x = int(s.split(".")[0])
        r.append(m[x])
        p = p * base - x
    return ''.join(r)


def convert(m: dict, base: int) -> str:
    suf = frac(m, base)
    match base:
        case 2r:
            return "11." + suf[:-1]
        case 3r:
            return "10." + suf[:-1]
    return "3." + suf


def main() -> int:
    print("\n\n")
    m = init("small")
    for base in range(2, 37):
        val = convert(m, base)
        print(f"π in base {base:>3}: {val}\n")
    m = init()
    for base in range(37, 171):
        val = convert(m, base)
        print(f"π in base {base:>3}: {val}\n")
    return 0


if __name__ == "__main__":
    exit(main())


""" output:
π in base   2: 11.00100100001111110110101010001000100001011010001100001000110100110001001100011001100010100010111000000011011100000111001101000100101001000000100100111

π in base   3: 10.01021101222201021100211111022122222011120121212120012110010010122202221201201211121012101120022012021000010102201002011112000222102220110010111012110

π in base   4: 3.021003331222202020112203002031030103012120220232000313001303101022100021032002020221213303013100002002323322212032301032123020211011022002013212032031

π in base   5: 3.032322143033432411241224041402314211143020310022003444132211010403321344004324440144104233413301132312342104201113210211420103320424312021121413112210

π in base   6: 3.050330051415124105234414053125321102301214442004115252553314203331311355351312334553341001515434440123435445203004500242234314025131145211002025103101

π in base   7: 3.066365143203613411026340224465222664352065024015544321542643102516115456522000262243610330144323363101130410055004102412535211655210553625150303312424

π in base   8: 3.110375524210264302151423063050560067016321122011160210514763072002027372461661163310450512020746161500233573712431547464722061546012605155744574241564

π in base   9: 3.124188124074427886451777617310358285165453534626523011263214502838640343541633030867813278715885368136538681688517621483015261781343583732478554878425

π in base  10: 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128

π in base  11: 3.16150702865a48523521525977752941838668848853163a1a54213004658065227350533715271781a6563715781334928885281912992063425270781275548269276978180640386187

π in base  12: 3.184809493b918664573a6211bb151551a05729290a7809a492742140a60a55256a0661a03753a3aa54805646880181a3683083272bbba0a370b12265529a828903b4b256b8403759a71626

π in base  13: 3.1ac1049052a2c77369c0bb89cc9883278298358b370160306133ca5acba57614b65b410020c22b4c71457a955a5155b04a6cb6cc2c494843a8bbbba9a039b77b34cb0c036cac761129b316

π in base  14: 3.1da75cda81375427a40abcb1bd47549c89bcb6861d3327c740cab809a52d0dd5171874504a5481cc915490bb577dc25630b527c8356122bad70131c04a8b2d1a1c9a5236312d9a357858da

π in base  15: 3.21cd1dc46c2b7e508484773e06919d1e50963db79c69739ea3731e79cde10a8ed4c630a83b9b5da464a91520862aa1a464dda50be4aa52c1c457541ed5ae5dd2da6a15a14cd9a051bd6c76

π in base  16: 3.243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c89452821e638d01377be5466cf34e90c6cc0ac29b7c97c50dd3f84d5b5b54709179216d5d98979fb1bd1310b

π in base  17: 3.26fag579ed6gf1413f0af7855fce249g1b2a9c3bdg24f4fg2b42g270f00g40cdb536c5e2ge49g61c609fb13c861db930f24b10e4ge680gfge9ad5fe5cb1182e4a3fdf3472f077g68a4f90a

π in base  18: 3.29fdeh0g77189360685db34f346af733gch9827f345h0bd5a47e3f0d962h9fh32cf2e9129g62h2h3a4b334a2167a375hh5f153hah35c81egbgb191cef90g7940779ff3273h0eg383g250gc

π in base  19: 3.2d23982975gg3c88d5h547h80d1c828132gb103h4d78a5if952ce3gg93ifc0829ceg62c96ib05di69840cg9chg58d1ac741a3a9a62ea2ddef0g1ei62b79e7i37hff679327f803gbd05ib8f

π in base  20: 3.2gceg9gbhj9d21hihb3egacb0361eb2bfb8h83987debh5180cfag88d2c627c3fiacdi7ddid6ec0d0fe3d9h4bd40548bed71641ag4hg1ecacc9dd1379i2d420c9j3aie2bj022dafh82e9f6a

π in base  21: 3.2k961edi5h86b6092ejba5ic88f6gafi01cfd6kegdfeae9636h1k6aea8d9k9jea44a5hkf39h76928fa87718jkg4fa6c3f3ed8akg42i591ah72g8a3h0a437d7g2508b92ca7ica0ei0da2kka

π in base  22: 3.32bek9a809gc6ci2dl3h0hdk921a02da797ff2f5g765ia0k5jlfdc80ia8lj2fifgbci035g3kf4j4ch4g9b66e0f1gkjk12d2i4k61kjle42li5gihhe6bak36fj6e62c15g31b6f39gakeg7df3

π in base  23: 3.35kh9k813jk9g9d60jj1570lia3cmbm3l2jkm49097g1ggi869m5724ck6h50jk0mgbb021c0f5576ef95abc5mgd334ckaf4hmlhf3fj2a6l06ma8cec7cc2kak94mgahk3a8f93cl6fb1324f503

π in base  24: 3.39d911bclk44ac77al474h7nmlkb0ee48nai8232m878e7ecl49g4jldj9h1931iei127b3iki93d9h34669a05856e5leclklkbhd0531i0ec91icbbf0l3ka8643afbg3g41fn2l8605fhk14k69

π in base  25: 3.3dc9fine6e7e492gm69fag0c0jo8c6143h8o0neo1o5mili18d7jb4a68b26m13h4egab7986c5c0f2443he5n41jinhoobn255d7en2ncke0f9jhg73j580i17iafhgnlm6dldc135c559fao27h8

π in base  26: 3.3higbebohjhe3db7dgj6ak3g6jk8hnd4g12a1ip8lggl63c4bck4nhe8g8o0blkpl41c89fggnbafbhd25m9f1oc66e79ccg8icii284a7l3kj2nj9fi1m7k5ecg90eio6femkfocb85i588phe3fb

π in base  27: 3.3m5q3m2dcpqodjngig99aq8n55dlg4iofl0a836df2p8j9acgaj310q7oc8hleqnb846g8kjkqfgijel0e81njachlm2ehgmqhcnnc9pkg41ll44bq67dg3l06enj1mbknif66359ahi8kcmlhchef

π in base  28: 3.3r06liojplra7747bcodkoi0q488g1ifcpqro1bkjnr2qpf3l66q15h358ddlge07483i4rqc09cqekd29j968gfgaijkip4fb60ch0o6coaanf7l6nlf34hdbh5g3ngcd92p00p327ohgc4mckdg9

π in base  29: 3.4328n0cjqmjqcb9i47beqaeh9n7rq0pbco4482cpikdajickasd18r92qn6pdqa08e8l3rh869bo3ffhhs0fnaiaj33b31a8ki9s12coo24bai4coea3e9ns383f5rih9loe813afd5cb9d7iq2hrp

π in base  30: 3.47d01ee07r08d7j8nhneef457glgcr9hfrhshj92asedi3hje8a0ljqe94emh39h7hdk3807o4p7dsrcembm7g6kpik34ea2l3oqiceap6h21cg1bh207rip330r1n414725sikf9df3i62m65or3j

π in base  31: 3.4c25oe856s61n7cuknr8g3n4hot1siiun64oe9rsot55jd8lb3be41328lariq622f94jssb1bdr86pr91ueqtclaj0j2ldrf343o16umhlgpbgojomjplgi9u662geutk6ojlhbo0gune3632fqjn

π in base  32: 3.4gvml245kc4d64oph8n06s3j8ii0ie1256fj3k085rt9hr2edi4kaa11sosd04rnnpa6djpkt466pg5c56rsiv2grkvo9ldlml3gi5si2ratj2bpvcdt2c8bkqcdvddc5vun5mug3bfrfe71lvmmk9

π in base  33: 3.4m6dn4ow9r5kahggcsmcev63k47rm05lj726ib3mv14u57vbw975qlurelv4ijbemfqidae3vmh18udc10l7is1g2o8qi4mkms27tibnihfae1374tburnoawmgwdcluvihtdbavm1spgdghuonh9t

π in base  34: 3.4rn5c8ianv8bnanx8qk2ovngfc96hnc8fkw1vxwk4wmgv6cmj30loskbj77ncv2bepces0scl2l2hxovljbrgmdrdqhfn42trt348nwqpt42khn6pht62vn1v4taisw7921pm0m1axosqdxp0cod0w

π in base  35: 3.4xfrgmtm53rwa23okl93476d73dcuy1sdoefofdj9sgis4h3kymy54pi86iji5w8pf1ek54g1tb4wnc43xsf0g3r3t8k5hbybxjjs0gtkdvuvfoqbjy7si33ahk1m9cr4ttra01lnogp8s8c5gghv5

π in base  36: 3.53i5ab8p5fsa5jhk72i8asc47wwzlacljj9zn98ltxm61vyms1frytci4u2qfra2vjaw70ch6j153p3z9zl55ukzl0kapwjygjou067iy9wnzdz9n4jltedtiw2b65acrpil9lj26st5c8fx8s5lps

π in base  37: 3.58V3FWNJB6U483QXYKN7VP8HOSS1MO4WRaP71Y5HHaX2SHUGaVRS7aSG4A8TTLaDFUVEMSFWBNQX2C9P4S2RLPGYFJC7FTPZN66V3Z0KAGBCVMABU50AFYX1XaWDLRWQ13HMXMH7IOXJB9LV8GTW4G

π in base  38: 3.5EHHZQF8IO9DZVFB6bIaXY31N5Y3K2KF0VA49SOTZFYJXRV9PBB6KBZNPLBARLa47JOMbVITOFA6F35QaCOZXQ8W8O3SBAHJDUHaEKIL0ZTEHOEU8DI5SZO2CL3V58bWFWE58CZa8F0N6bWO4NGF1Q

π in base  39: 3.5KE59TGIPGAF1AM8PNWQVUAFD6CKI4ZRWbA2UOCEAOH01IJAE7aLT7ZZGLPQ4RERHKSbP7bbNTFQF0JUMB99GH7aQKKAG3PaPKPAHX9011UQ8NFZDNbY5LGEL2bJFbcbWCDR73P58U11RJXQDGFMYY

π in base  40: 3.5QLb7T462UDb13PQJMDO8CF98Ba0D8Q6MIRD1YXO6U6C33RX4ESd2X1MYYOJbTIBC05dAGYN65HUPJ93KSSJEaHcUPUC02EUdMXTQBBUMVAHRERb9SVIBCAKL2ZC5NZE8WPTSX1L5FFKLC2GTK46G1

π in base  41: 3.5X0SecCVDLeWdc3Q2cAF957aeF61a2EXcAKGKJHT51SC3dI0ETXX2KJee7U18BO53OKaTOBSY4DMcBIC7eeVHQ1WSZCbaIQDXJ12HVBU8XWESF0Fd9aU7XaCJ6TXVKAC6GFR24RB89VEJRETD12UV5

π in base  42: 3.5dWDCEBH25BNIR6aF1QZI3bcG27BKUCd1DFIZVRHRWbLWK1c5950fVeEY81183VIW22RYPaNJ6CMDZ5WKIEdfMMFDI24IcEUcGIdLGE16FR52a4WA6WA30IfJEINdX27faXLJa3Lec3J0MFEJ7IINC

π in base  43: 3.63YQ4NHVKY4DdRM40QNNFGCHQCbEORGPLAbWf8D6JYYD82dO289fZ9PDOO21BA34caJL32P1WJ69HAN5QIWF0JKTG5CSbOSE6JT1CQ4CJG4dCANdTX1f1bFH24OIHLLP9YEAVIFGBS0A4Mdf54VbYP

π in base  44: 3.6A5IbY6T6gWfR2C8OE7bVKbM3U1WKRM3J8KGh2gfNKOFMdY6IWWeUgFISfb0KGaG8LLQRCCRBOIH7FQI4dXOUPbEAeF6TDgWTDLTZSBEOH1XFJ1a3A5a5UQaM1IPeQQVOUNh80NJ8FgCHOb2XMadP7

π in base  45: 3.6GWSGdSIVRAMGVQhHMfRYILUNFDhSE8C3OOXNWBMTOUEZON1YDUVQRZNfgPDH0OePZVVO7U503Xb9DHZQ1VaHhc7X7iVcHAT68TaKBIhEP7SGZI19dP48SWYc8BGKVCFADDgVS2MRHMINQDd4Gb6CB

π in base  46: 3.6NS2eEIN0XAhOhNQLbS0DMdfTJTiMUVHWbiH2c85cJ9ch0e692B1jCCeFFZOXgE61K1DH6g9PJVHDKWZDfEVM0aR6EEiZPNXhVKcYQ2g2MOYjC2SKHNSM5YGFOKhLiRUTfe3e7K0VUNJ5MPhaMeFVY

π in base  47: 3.6UaQk62DkNdFFE8j9Mi8c85iEWAAJO9A5VbQQYXAVSeGZPcXO426LPEd5JZPWUHLeSUF6JNTMTDZdg4EVDU9j4aX2HEQ8hWCf8SRUPOSAcCYWY26EeaehkJPaW1ji2DShKejTaC3CUSe13N4C6hfj6

π in base  48: 3.6cB0XkaekIQY7kWKILlSggbE5bQOhJT3EBeTETbD5FUb66O5VU7B1MWZKCERe8QWAIPdHgiTLcSQalMKaBNX3BZfAA9jeeVKkA8N7V4lkKhIJjAgU2H6OaUPAGGIeNTKOiZSdjMjd0GiT26aXNb2Oc

π in base  49: 3.6jlBN3hP82jSGWlGKkQEl2SCdVFdKV7JD8dfbE0KGVh3LBVHRM1A475Z47I9cb8lb7eRJC33MIII2h0Vj5kc5KO2TNhSF0X0jS7Wmb70QS0ST8O9CHcHdOXSkKY80JUOFZA8D8BcLZQTWE0bidMIBh

π in base  50: 3.73n44CH1Ejl929OH85ekC953dlFRMbVf8HYgPJ054AiT4JYeUNBBmMAM8kflCQU17df7JGJUYObiP0eM2g9m027PJk7NJDVINOgQhbD30nh1ahjEbdYM99h01FDPNmSUdjcT2l9l1GKBNAQV2m0W8C

π in base  51: 3.7BEKch3bX06cl6F98e5ER8GLDaG83BCc9UEoSTfnkCin4n7GDGdL2R2oJF4Qf9hLFWGFmOMgIjE08PDLa9PA8JRZ2YC05kBK4bXLOIS9KCX6bjCFmA4d4CaPLLoFUCe7ehI7bXjoDJPMOOZSiT8Ghb

π in base  52: 3.7Ij35fLQhMnM0g3QlQA1aj0mCWOKQH7kK1TPj66jJ2BK6WKERMcDAiLKoIKf6oLeCZoQe2mMDb6VopDGe7J8UofLEoN7RdO43O7hKDJ3HpojD4Kf5MNCRjDc8L9U43IeTkM85KLKUZQFZi0UWQSRQl

π in base  53: 3.7Qcl7UK0WGhjFOAmHokljVK6dHjpNnAcJLJ3hdAPpBQ3HPCqVikdiF2iCCl9NL9KXNik9oEok5EK9LeMlQp38Pd2nL4OdOpNh7Shh2eK9cFKngohPg8HYMbZgRSqNpaEje8nE0c6I43RDgnXd8n049

π in base  54: 3.7YleE9nmmqR1i7iK95jjAiF2WjbgFZOZl2Xc0AU3aFiWAcJB8Cq4Jj7Jlqc4SLcSDOFKQfUfc5TkqlKSljNGUl62Gnb7BWVfm65a91hBBOAkaMY6O7ePQSnK7AargH2SGlm1pZIDh4eNnomPAcDHbO

π in base  55: 3.7hHQF98fT26s90q0UHpWCLilQIUL5AoUjhDOdc7b6QG6JEPVEGIqcdXJeELmHjPRi6A33MdlFienk8W13qJHEbXVdsJJlrkZC1W3C0nQ4NKE968OHNPV2pRPbE9Sf2ihbgCiiGeW7jFjaq0MU4nFrG

π in base  56: 3.7q1qLWR3fmkLKVMsSAsdStJTZO8l35XRLjstK49L56DbBTWObhfKY5R0hBTWaY3pHUFQGe4Z0fFfUfPrWCQVE54H31SWF48BUYberF1enHNFagPUBWmrfqOKipUFZQk9LfA42IJe3mbTd4tU9ORLTi

π in base  57: 3.841tAudDBX3tdRXe30U0aljPcT17SDPETUZIqE0hXBSDn2hT2VXUGf3nIr51RREccSEQpen2ejkBj6BTMskQEeL6Hj155PmKlcHjEshDiMaIRDmHd5g6QStXJ30Ltk7S2eeiHbKeUq1ql3QYaVhp2g

π in base  58: 3.8CIOeRvsOV4I54CVoX90Pe0DB3mQ3Z57fqChoGtjnZI4bcIjIqb3ZRjNP5lT21oj69n2TF8PG46pdqjcokGWnG5QJultQfA0f9rPhOA4IKuOYRl5kdFO2LokaZuZCo8hm49GiQ1aJJvsI0fljiUKgL

π in base  59: 3.8Kq9Ha3fM3h7KRMUYPgbdEJgE3N1HQpFs7OYUed2gHWrgFk6XrivphH6YR5Ed3Up9ArqWk6QXpk4uKZ5Ff6dUKQVgTT4JBIeDQcZPGF6Xg3dJ7EJZ7l3nGSll8lq1aTXVslwQRlW5uHNf0l128dCJs

π in base  60: 3.8Ti0lPr7OvaHh4T7A3fHqaCEaipoFX7Nx9DmMCLjMuldiSbwNLBuXMegV663kGq2mXOcXM101eTc68xDf2SGhue7Evnw2FG1Fv3OxIJD6loVBEdNt6DdDC6tLWWQoG1ivJZ1H0Cv5WqI30b8vfJGwn

π in base  61: 3.8cqpMShW4b6Whksi22XRsyqOlV2pFXMbHnteO0DUMZA4t39NSWljmPEs3giYM29RaqMpVuHtuGQOqBAtQxCcBEceL4f4yTfGcN5USYf0cdrQTTGIwnBO4nxrtP3nBTnyDumZsDcNgUyFlKVIoAQZOW

π in base  62: 3.8mHUcirZ3g3aaX5Bn156eBkfOx43HPGx8baQoXfOkoX6TVMlsvEFNGhx0OC2WbXIMW3Z8Ny4xFMLeK32IdE2ZJxYrmnciZxW7TDZNZK5H7qeWEuwN6Pnixsg3uTZm6IamrvGuPtwX5sOxpu9mGyV3k

π in base  63: 3.8vzpYeVTxpDEITkefvY6sg1hf46flk3XDxzk2o4k7aWzIZBEvwqaSLffbkFoDbUqf8Kxc5SHMXqkEr7d5AmcWBuy8WiRU87rRA4oUz6cI1AzNJoDIl6jMq6zwltu4fRrlU0HTeJVrEygHvJOwyq4qF

π in base  64: 3.93zgY8MZ2DCJ6Oek0t1pHAG9E28fdp7G22xwcEnER8b5A27cED0JTxvKPiyqwGnimAmfjybyKDq🐁XDMrjKS95v8MrTc9UViRqJ4BffZVjQml🐁NBRq1hVjxZXh🐀rg9dwMkdoGHV4iVvaaePb7iv5

π in base  65: 3.9DEvNYPVmhEcv0gMMjidmqHmb32b1hZ4🐁bZ9Ow378WptDJdEjHu7GyVsVS4oNxvKOt85YGhwGf🐁R🐁dAT6w9Hun9n6twzszLMeRSeSmMWS9ftIFuumkxBcF44j9bgGng🐀bfVW88kf4NtHcqf1p5

π in base  66: 3.9MpLEeTIDaH12EMn7🐀LJ🐁6YfNWHvy54302puzmhj🐁dUTMvgCSbuICkF🐀2GDwLPrialp🐃kLIEt94🐁jjkW2wDTvf1TgmtCtg0mMVF3iXGQbnSIfGlwfsbCvMtH🐂W🐁zpHBwO🐃lbVMT4O🐀r3

π in base  67: 3.9Wetkco9L🐁LtKLT🐄OYeB9d4O🐃jerO2X4rfb🐃JGFMxEynAKr🐃Zdnyv1lFYHxukJsB6THBnxm7nr🐀wGC2gx5hjUA🐃A6nstTxEg9ghL0pUjAVvrKeejh7aK7ZpBKG6🐃8NMtaw5🐃4a1tbuPUa

π in base  68: 3.9gnHq2WK6ZJypMjBmMC54NBsW2RdrOrkyG1ZQqG0tSZntri2jqI🐂ThKbB5j🐅bG🐁XOtxXl3dl80tyu4os1huF8RSnQB🐂qyxz🐁7pb🐀SUR2nw9D2WiJa3u58Lwmfawqzcrsm6WBkQHEIPhEbetU

π in base  69: 3.9r8Vu0C4UpGq🐀gNPhQ9NpFvYp8nrQjm🐅ej6iWrCu0d7oS🐄FV14ax🐀22ZNvq9xK3rw7xHS5rRI6🐃dGr3ZEwWAJXYsz8ncAnQZ🐃Y🐂vhEnoc🐆🐄UaxQf4owM4yCFzH🐀🐄adaWurIC5dVv5qT

π in base  70: 3.9🐁uJg🐀C🐇Pc🐃JMzEoZoAr5🐄rJJ🐁🐀ct8FVrO7PUWqTT0🐀N🐃FY🐆vqXg1rPWbnON🐀RIyOgLKKpRLOmHPf🐇Ul🐁🐅Azqcj5tpll🐁62NuojxmemAmVjJdKuMaNp6Ja🐅GkAilhhOZ🐆H🐆🐁

π in base  71: 3.A3seOY🐇gs4SOF20🐁jjWrw🐇NjyHZeoFI4🐈🐁qhF🐄i6oEPv6BG🐀winSk🐂mmSPW🐆6b🐁wwNV🐃jL73lIb🐆🐂2Y290YAuGL4bT8vdiaeYw🐈skcc2ff8🐄iigFOkJK🐁🐁Qi3🐈NTZ🐀🐁D7🐄

π in base  72: 3.AE1Cf🐉XUmaXkYeFDzcuLz🐈ElF8eNdLk🐆1M🐅TtRGYcgRlEYer3C🐀SpyQDGTmTaCC2aIQ086p🐅Uz7🐂🐀EMij5🐃7Sl🐈🐅7🐈7🐉jB🐄🐂o🐄gnFbyxn🐁A🐇m🐃EnJX🐇7WHh🐀VK74wwyd7

π in base  73: 3.AOd🐇L🐆F🐃Vi🐀🐅C4qFtOwK🐃Q9KTuq🐃vROdNIiep🐈SL🐉l🐆GvcdiK🐊I🐉19iq🐉FJgnNFN6🐁adQcA78O97a9y5m🐄🐂q9IBFekqepVY9🐅🐄5🐊aT3BK🐆kEuAQl🐃D🐇4CVEBfNBCRnM6

π in base  74: 3.AZQs🐂GovSLVcvLEnsAs🐂rIO🐇c5🐉ev96Afz🐄🐉gKa🐄ncdhsU🐁3VGGGFQc🐇wo7JK1🐁rnOr🐋zL🐀VC97nNoN🐄C🐄gFurzurYpmIYvaVat4🐃SP8LuxaFM7VP8🐉2zT🐊🐅6CFYXfNkXZFx

π in base  75: 3.AkYU49Nrww🐆NNe9Iy🐁sUPpg0NzJtBk4kLmvyqWpKHreMr8🐃ICy🐋YJ3uDZSh0s🐅f🐀0QTE42j🐆mX1🐌lCf🐅9🐅g🐇vuuJsQVv🐇S🐀2x🐃CCcOwsh🐇🐌XJ🐉fM8🐈MrVayed🐀xV🐉🐈soG

π in base  76: 3.Av🐁x2HmjgqVXrsK1qs🐃D5l🐁🐉f5qhnu7XJ🐇7W🐅h0tJ🐂ccLW3🐆bJh🐀Q🐋🐁hCs2T🐉eYyeEk5dVcU6txW🐀QOezOG69Y🐆l🐃Cp8🐉Sh0F🐅x0ERZrkVCBOy🐂MuDANoah7xEvrCPOc4j🐋

π in base  77: 3.A🐇ctRbNJfXElpIT6Pod🐄8VFYvW🐀🐈tD93QuKkXwd8🐉RHy2sGDKFv🐆s🐊z6OwZOPmRaFG4ehx🐅🐁🐇E1🐇LaZOF🐅🐍pfFyj🐀r1GQSdwRtkZU🐈I🐁4🐉s🐈yT🐄VUK🐋BBBy🐌u🐁🐉🐍🐀

π in base  78: 3.B3Z60B3G🐃grVcFdVTu2fE5D81qYC04iR🐏afj🐏K🐀GZ🐈2🐋w🐋wRAs🐉j🐃NnyYowD🐂s🐉🐃h9HQ0p🐊🐆m🐇ici🐋JS🐇hcupRcL🐋L🐄MgBx🐏GU6🐆L04G93mMS🐂KxRZ40ThhowU🐏7E🐁

π in base  79: 3.BErtPzmADlp3LE🐏pjs4🐄5OrGKscCaWC04uE8B🐆🐃1TMyMs🐈f🐈9gZ🐀🐎9🐋e6🐎2kHcFCumdkzR🐂i25nhnMyZAi🐇SW🐃CfP🐌🐉wT🐏nseJjR🐈🐎Dc🐃LgzYiKmPD🐐qT2s6X9dfF🐄Ooo

π in base  80: 3.BQFZ7MpXqad🐁v🐁JyqA🐑r48xn🐊x🐎0Y4b🐑5BzO🐃Y🐄VTfx🐊🐇2cWLK51z3lU3🐃FL🐑SR2XR🐊PHX🐀8ydY0🐌fm🐏🐇EdBUb🐂p0bC🐑k🐇🐋N5l🐀gGTwBUlxrqvYzLouv🐂🐇l680🐄gk

π in base  81: 3.Bb🐒Ba🐅c🐉🐐fG🐈t🐄9W🐌🐏FnmmgOlRAOTDjQZw3dnFU38z🐋T🐉🐂r🐏X🐋Xm🐐🐋🐀🐏GuD🐍1lt🐉CdrYTh🐏n🐑🐎N🐈UPz🐂S🐃SVZ🐌7e🐂L0w🐏vGc🐀pFIt5SE🐊QMM🐂🐎Hh🐀0eFA

π in base  82: 3.Bo5r🐑🐋🐍🐄gt970O🐎🐊I3bGSj🐌o🐂u🐋b🐂🐀🐄🐂Wzk3🐏mRp3skcUs🐈🐈🐑vv🐏c🐊s🐓XE4y🐃I🐓🐄🐊0x8🐇🐒s🐌Y8🐌gAul🐍7yV0🐐WxrkZL🐀Ak🐎pAfAC🐂🐇🐅w9RG7ZoM🐋JY

π in base  83: 3.B🐀Z🐇oJ🐑IX🐔🐌NxH3ITlZBs9uPNBrNG🐔6🐔RSiEYuVb0🐍S1sie6🐏6🐓🐅CF2anl🐑rQxp🐃🐐p6S🐓Grx🐓🐂🐍Y4oWBho🐄6z🐐36P🐑trFl🐒🐑🐈fe🐄🐁lLiae🐍🐔9🐂4CHUZNyAU1🐊

π in base  84: 3.B🐍6iw🐋V🐒🐒P🐅EgEMIpS🐎Do🐎QDN🐑ov🐐🐃🐉gm🐃E🐀🐏🐁🐀🐊T🐈5dsiYn29🐐tl🐋🐑🐎🐐🐀zY🐑🐍🐂kuw🐔🐑2🐋gOhT🐉sudt🐓M🐓LLZ🐋u🐊otzYY🐌0j9🐄🐎brV🐒Bd🐁U2hd

π in base  85: 3.C30o17cDmsIW🐒A🐃5M6🐃🐈Ol🐊9F🐕🐖nWAZnN🐃T🐋j🐇3N🐑N🐏K🐐JRiEHUY🐒L9JZNr4ciZvgm0G2🐍A🐂lN🐈nJx🐓rZ🐆🐊x🐇KI🐀🐕0🐖O🐈G🐉HLQXF🐔L🐇TL8tPsz🐅LlzWexQZD🐃

π in base  86: 3.CFI🐋xaXz🐆2🐕5🐑y7A🐓🐄9🐇uv🐑🐊PV4p🐄NAZNEp🐎9j9qDJWN🐗hMh🐎sEw12🐍🐌1Vgmj🐅yB8wm21r7b🐍🐒🐍🐎P🐐ZLQk🐈VWi🐄🐒🐆uAA8Acg🐇🐒hS5🐎9m🐀0b0iM🐔R🐗UXm4🐒

π in base  87: 3.CR🐀GOv🐒iM🐓🐈acGf🐕UcP9cdPY🐇is🐔g3x🐈🐖eDj🐒Zst🐕🐉8🐗z🐀🐋zmGM🐀🐓BUwQEUXv🐈🐇xwc3vA🐏🐔🐁Qwp🐔MGN🐂Z0fzJ🐕h🐗🐔OpE🐈F1j🐑🐂y🐃34ASF🐉L🐔mgUEQu🐄3

π in base  88: 3.Cehb🐂🐋y🐊nc🐂🐘N🐖🐃g🐎xn🐙geL4🐑NiM8lATMX🐓t🐗Aj🐄🐖WM🐕g🐒🐓🐐8Qk8lL🐑🐙🐆Y14🐐🐘JF🐌3H8iZ1pJmXxowoh🐕i🐋O4🐂1🐄🐆O🐗zDxLE🐁zef32O🐀DN🐇Pi0G89jxH🐌

π in base  89: 3.CrncZGv🐆uwyo🐕🐉q🐆qFEawmJ🐐ke🐄sO🐎rTZV2🐈M4LR🐕🐉lBdA4PRvAkFiI🐎hFy🐉🐃🐍6E6j🐏f🐉pk07🐃8🐙lM🐚U🐉TRhbXH🐖🐌🐔q🐗F🐃PlVnphaUNp🐆YeWM0🐏🐊CoLSGzz🐈G

π in base  90: 3.C🐄🐓40GbH🐉🐎🐐COfW🐙🐙Yn🐛na0🐇DFo🐓6🐁🐆z🐙8dJSO🐉X🐍🐐rSnJPE🐓P🐚V4🐑🐚🐈hUY5sRpAc🐄9F3🐁N🐉fB🐔B🐐X🐂🐅🐀🐏8🐖A4vrBpAHJO🐗E🐚C🐐6🐃zSGBt🐍uVMzImX

π in base  91: 3.C🐒mA🐁jm🐋98z🐂nvTw🐘2Hf🐛hEHr93d8🐇🐊Z🐌RNev🐙🐏🐊7f🐊🐇6🐈Yd6🐋🐑🐄THI4vn🐚8lEPz0k🐜8eQo52Da🐜b8q🐚M4Cc9🐖4🐌🐀🐐1🐜Xx🐃Z36z🐗yKAD🐛H🐛j🐕oF6🐀R🐌🐚

π in base  92: 3.D2ek21🐆41🐙🐀rP0🐜🐕cQOKtj🐝B🐒GbG🐊ibO🐋🐓🐄ryPY🐆t1s🐑9UtpMuXwF🐙TCF🐕uW🐋Vh🐆🐐A🐍VfqGEMQu🐂g🐙EY🐂Pslx🐚nWJ🐉h🐚🐚mqInWT🐎🐁🐊R🐚6bCm🐎DZ🐍🐇k4a2

π in base  93: 3.DFx3🐖🐍8hYMWO🐄7🐍SpFwaUNcYA0Cnr🐙uwRke🐄Z🐄u🐁🐞yHV🐔0Do🐈lCobq🐚Lt🐊ui🐗l🐞rGY🐒Zf🐒🐒🐆DU🐗🐌🐁🐀🐘EBzK🐓S🐅🐊Nqw🐜t🐒n🐘4🐗🐄XzwW🐖QWi🐖ul🐖frt🐛

π in base  94: 3.DTAt🐄BC9T🐀🐍🐍pc2Y2🐇EoV🐜CsVpBfBT🐀🐌r53🐉🐅🐃0f27e🐄xJ🐋E🐚xIt56xUBaU🐃ay🐅🐏zO0ep🐊1n5PUbm🐏🐙🐎🐜4F🐒🐛mqyD🐄G🐂aLH🐂🐉kD🐄🐜🐛JMmq🐊xjd🐎FEW🐏1

π in base  95: 3.Dg🐕0CZ🐚npIt0🐁e🐓🐄🐙H🐞ur🐖I🐇U🐆🐃YeNZP4M🐋🐙G🐒uYwkcPE6D🐓K🐐🐑mWo🐝GfXDb🐝byPY🐜K🐅Tc11🐟🐎🐄🐠N🐅o🐎Mb🐘K🐄I🐇wKN🐗1qDXd🐃🐅🐒6a🐇MuPS5🐊UqN4E🐋

π in base  96: 3.Du🐚BVHDNb🐛🐓🐊T🐝x🐓q2A🐎P🐠🐔JhQLl0🐔zS🐋5z🐆P23🐇0oB🐑S🐌lN3pev🐏o5c3Lpm🐉O🐞🐏T🐉🐑6🐈FVR2CzKq🐃9Uc🐉🐌ou🐗GJ5D🐛zRk4shoKaPwjL🐌7🐋🐚🐕🐙🐅sBG🐄🐛

π in base  97: 3.D🐉N🐎🐑P🐚2C🐅🐜LmHqgsV🐏pZA17🐛🐐k🐅0🐜🐖dJ0🐅P🐃I🐆6🐛YsZ🐋c🐡🐘🐒S🐓j🐏BPOx🐙teLY4🐐B🐊vh7kWel🐆🐞7🐋🐒1FS🐞🐋Cg4🐝I🐐L🐇🐡wq🐘Y7ZbT🐚5j🐐a🐚🐋Q🐉

π in base  98: 3.D🐗🐕🐗qq🐓🐊Aymn🐇bZ🐃zKJ8U🐟AhxJw8X🐜🐝p🐆Uq5🐐🐖🐂MR4m🐗🐞🐃🐅vsAt🐢🐂🐟Eg🐀🐘X🐂M7Z🐇🐄🐞🐅🐛ydX🐖🐗🐙nI4Zi🐈3qUc9Gfzpc🐓XJI7🐘a🐔rt🐔uLzmAz🐒OL🐠

π in base  99: 3.E1🐌K🐒2🐑j5🐎v🐎🐓hO7Tgx🐑T🐂F🐄🐢🐇C🐊LbBxlR🐕f🐅🐒🐞1n8🐃sXW🐛iG🐁🐕dL🐜🐁P8L9FQfgT8U🐎🐎IESma🐏🐏v4GfhE🐀dT1V🐍🐕Aqk🐘ZJu🐊AiFKT5AjcQjPrde🐈🐆f🐗🐏

π in base 100: 3.EF🐞🐃Z🐛🐑WckQhcW🐑oS🐖J🐉🐇d🐟🐍AwK🐣nixN7🐓🐂6S🐀8🐥🐘S3mPYLH6🐑🐔E🐒🐘pW🐔U🐄l9ciy🐡owMV🐊rxe🐓SmBHj2🐖AR1🐟🐗LAtx🐂kM🐠🐛s🐟3🐓🐢iS🐓9🐍🐄xXizSlu

π in base 101: 3.EUd5Vh🐁🐛ugGyoJF🐃🐦k🐠🐖u🐡aMt🐄Sg🐄F🐤P7🐜🐡P🐊xB🐋z🐚🐓🐓W🐑Zlx🐕🐡d🐦H🐄pmZ🐠🐤🐚🐇bV🐡2aX8F🐓g8ERjh🐣i🐖SvSl🐊3k🐌31U🐇4AP🐠🐖🐥6🐞C🐜🐈jb99c🐢🐜

π in base 102: 3.EjDQJ2🐈n🐅X🐓6🐅wzS🐁🐝7rd🐙🐈Nmh🐧Hoz🐒zF🐔8zM🐙5🐈c🐄wjmIp🐂Y🐈RB🐋🐏q🐉🐑🐒🐤🐖u🐜🐒🐕qo2🐉ZG9🐉L🐐T🐠🐤🐢s🐀🐥2🐦🐋🐆a🐑0k🐡🐑f🐊vb🐣🐋🐓🐖MzL🐝🐝

π in base 103: 3.EyGB🐟IsbE4fEgCG🐉V🐍2U🐀x🐂Zf🐛YbV🐣75🐤uW🐢🐜🐣🐟o🐂A🐗s🐍9y🐓🐞DD🐨SRCWW🐈lV🐊9🐒i🐏🐣c🐗7tiDO🐔S🐙L🐗🐇8W🐜🐘🐐WfP🐇Y🐎fNcD🐌r🐚W3IokfR7🐘4🐒6🐞🐢

π in base 104: 3.E🐍mn🐓oo5🐛🐧R7🐍SV🐇UX🐨ow🐁🐉A🐝N🐔h🐅jKr🐗🐘b🐐JghmHaOP🐨I🐧8BhJBh🐊8deq🐩🐄🐕AHB🐐8yyZHBf6pgk🐖wg9🐟Mp0🐆3rGX🐆U5🐜V🐗🐒🐩🐏4sEu🐙🐘🐡🐗W🐗🐞🐜7🐓

π in base 105: 3.E🐝6Ku🐃L🐖🐃🐒k🐒🐙🐨🐖Y🐃🐁🐒J🐧C🐧x🐚U🐥htw🐡🐗RqC🐙nv🐗JAIrq🐚8ouNL🐏ko🐂Y0🐞🐛fn5🐣🐁FY🐉OrWFrt4a🐀j🐗🐟BRL🐗🐪F🐦M🐁cyrg🐄D🐏8c3l🐚🐛cgPbYU🐕3yw

π in base 106: 3.F0🐥CUaHoAxWEvhSKHf🐈MI🐍nmgr🐤3u🐘G🐁J🐥gG🐚🐀P🐈eQ🐀T🐜🐋3V0🐨n🐤🐐🐣🐍m🐁7c🐀🐄fhESD🐉🐨🐀🐅JF🐓Ni3uASfNM🐤🐑8🐜gs🐞🐏🐠🐉C🐙KSHs🐅ONn🐦🐑EO🐃JtA6G

π in base 107: 3.FGA9vo4🐍t🐬1🐊🐁99Nj🐃🐤🐞qNz1🐠z🐫🐞🐏🐔dyKV7🐆9🐞7yL🐗Uf🐉ff🐃e🐆D🐆🐩🐘HUmM🐩🐨🐅Q🐌QQBLK🐤b🐋n🐁🐪X🐙k🐁9ZW🐡🐊xm1🐃🐊🐣6🐝🐊🐏k🐆🐀3🐝🐇gHIqf🐋U

π in base 108: 3.FVv🐪L🐡F🐦H🐖🐆🐦o🐁Q🐈C🐆🐖798B🐙P🐂🐂🐦🐂w🐨Ht4🐀I6🐒🐉zMO🐄🐠🐞🐉R🐋4sv🐒T🐐Qd🐠po🐖j🐐🐈🐦🐫🐉Ap6🐦🐩🐩🐉s4🐘🐒5G🐝PSGP🐅🐉z🐗rQ🐥UAx🐂🐞sxD1🐄e🐠

π in base 109: 3.FlS🐂🐂t🐏🐖🐕🐏🐎d🐟5🐎🐎4🐪🐦POCN🐡🐛l🐒E🐊ZI5HY🐣w🐍R9cix🐍p🐌🐧Cy🐔🐎🐄l🐏c🐞🐠d47JA1353🐇sk🐄2🐥jc6ILcP0🐇🐅🐈E🐛0iz1🐝8gnwXHd🐧e🐫mKmb🐌KX🐚F🐚o

π in base 110: 3.F🐁T🐜jaK🐉H🐚4🐡w🐟Ytwd🐄🐠bS🐠🐌🐚3tFs🐩3RGf1🐓O🐂d🐈🐂KYV8KH9t🐛pI🐪x🐂🐉Pbic2G🐠🐤🐛🐪🐚🐌s🐪🐗twg🐆dpW4🐡ajLU🐓🐑j🐨Y🐑🐕🐯🐏🐗🐡🐜J🐐🐕🐅🐘🐔s🐦

π in base 111: 3.F🐑🐀t🐗bm🐨O🐛ySH🐟🐓rYZ🐓🐇jL6🐪yOu🐪🐐PywZj🐈🐆🐃🐧t3🐕🐗🐜🐃🐠6h🐘s🐗aWgJ🐏I7🐩s🐔🐒4eq🐪🐎kM🐃wvM🐍🐠🐠🐀🐊fR🐗🐯8f4DL🐄Zp🐃🐓Sr🐓🐏P🐀🐐🐗Z🐬yjP

π in base 112: 3.F🐢FsI🐀🐪🐁kmDl🐤🐣abiL7w🐜🐎Tf🐅🐈HIf🐕zDW9🐠🐇🐩R🐝🐄🐫V🐊kDkT🐏7GJ🐬An🐩3o🐑S🐨G🐖8r🐜rw🐇D🐝🐨dF🐋🐀GK🐯W🐄🐒🐟🐡🐓Bf🐄7🐩GIo🐱🐗🐍DOXH🐪🐌K🐕CF🐥

π in base 113: 3.F🐲🐲🐇v4🐟🐏DDqQ🐚r🐱M🐫sB1🐢🐕🐮🐇🐍MP🐃🐉gF🐔U0🐭🐲🐋M🐁O3🐆W🐲🐤g🐛b🐁🐥e1🐘🐉S🐖🐌🐑🐍🐲Gyx🐊QB🐇I🐂gdu🐑🐂uTBNS🐫oOwECHa🐐🐟🐕Dl🐁g4q🐮🐘rP🐦3U4

π in base 114: 3.GGF🐗9🐠5🐌15p3Zjpn🐦🐔🐎🐆🐁🐚🐆🐩🐊🐝4🐫🐚i🐳NkA🐂F🐐5🐒🐪🐖🐚s🐮SJ🐠MFsYrA🐆🐨p35🐰🐦🐑vi🐳🐊o🐬g🐮🐄🐛v🐳Ac🐈🐪UD🐠🐏vUW🐋🐇🐋🐲YSX🐕i🐜fIQq1🐟MT🐰

π in base 115: 3.GW🐂🐕🐇🐯Wu🐙🐁🐳Uojf🐊🐫hN🐍🐴🐏🐴🐃M🐟🐒bR🐃🐎ay🐎🐁1🐒RkuKrl7p🐔6J🐦🐢🐆🐃🐜H🐁🐤🐮🐜🐎s🐩🐕an🐇🐊alj🐮🐕C🐆🐱HJnD🐆🐯🐤z🐨🐜🐉🐎🐇🐑🐙🐛d🐄1🐙🐛d

π in base 116: 3.GnVlJp🐮CYaE🐐Rz2🐀sm3🐟🐣w🐂L🐑Jp🐪🐂c🐨L🐌🐌C🐆dVzmgQ🐒DV7ov🐬🐇🐒🐊H🐀b🐓mV🐥3🐉d🐖x🐵🐙🐞🐔🐫🐬🐒n0c🐇🐵🐜🐩🐯Hf🐇bO🐖🐤🐘🐖🐯O🐰TqDb🐨SB🐇Uihu🐘🐤

π in base 117: 3.G🐄U🐌Uc1🐮🐯thv🐟🐶DpZAo9🐚3RgmP🐰🐱🐇🐘🐤🐜🐝🐵E1i04🐣🐈B🐬🐁🐆🐊kLi🐓🐮z🐭🐠X🐕🐣vLv🐅🐑I🐇vD1🐎v🐕2🐁🐥8F🐍🐈🐇w🐳D5l🐩🐅🐟🐘o🐠HFzSN🐠🐴🐠🐭🐈🐝🐜

π in base 118: 3.G🐕🐁U🐝🐄1🐛🐘it2🐓🐩R🐦🐞tpchVzx🐭g🐦🐮f🐮🐠🐗🐶kv🐞m🐢🐣H🐧NXo🐜🐅🐬🐐🐲F🐝🐫🐆🐟🐘p🐊S🐝A🐴🐘🐊🐑I🐦Lf7🐥🐧m🐈DC🐐Rf🐃Q1🐐🐔🐪🐃🐏🐪🐦🐠zu🐤AI🐬B🐡

π in base 119: 3.G🐧BG1A🐐UZ🐆Ft🐊🐍s🐮🐨🐭🐧🐷92🐑🐍w🐣🐉🐷DC🐧🐀7C🐩03🐷🐑W🐲0G🐬8j🐞🐯🐜l🐂🐓fj🐐🐮🐗8🐆9GD🐛aLim🐗🐫rSK🐖qV🐣5yZIf🐐E🐭P🐪🐗🐃🐴🐬🐯🐗4🐢🐏tS🐄🐝🐫

π in base 120: 3.G🐸🐲C🐏🐢🐑🐤x🐅q🐞🐆l🐄z🐣PvN7🐇🐄Y🐣n🐣cs🐁🐸Hb🐏🐞v🐙pz🐐🐪🐓nSXm🐘tmJ0🐙🐞GGw🐲34w🐯y🐀🐑N🐍🐹🐘🐌s🐗u🐨Eg🐩🐛🐘🐷Y🐪w🐝Px🐎s🐹🐡0c🐶c🐮🐩l🐑8Q🐋

π in base 121: 3.HG72🐠🐃qvcNv🐂🐖🐔Vj🐝🐠🐌🐞🐢wHhLxNX4🐉🐚🐉O🐒tweGTI🐛🐶zeG🐗Eb🐧🐢🐟UKD🐮M🐇kv🐏🐗D🐔x🐜🐍T🐍🐗J6if🐅🐡7🐪🐥🐔Qx🐉🐯🐢🐕🐲🐧🐡S🐙🐹🐀xHpxUm🐔🐷J🐵🐫

π in base 122: 3.HXu🐛🐯7🐓bft🐚s🐸5🐝🐱🐋🐹X7🐔🐄🐱🐵🐰u🐫x🐵🐣🐂u🐸🐺🐺nh🐧i🐴🐐B🐶m🐢a🐲🐂5fnRoYZC4J🐍🐜🐊🐝🐸🐛sO🐇5H🐙🐶🐷KK0HQ🐑s5🐜d🐏gtEZ🐓🐋🐆🐏🐱🐀J🐒G🐌C🐯🐶

π in base 123: 3.HpJB🐭7🐷OKd7🐬🐎🐏🐻🐠iK🐱M🐅🐷2🐨Q🐺🐣🐝nNUsr🐥X🐁🐤🐉B🐃🐖8🐊XP🐬🐦y🐋N🐶🐖aX🐹🐉🐺3🐠RSn🐑OR7kkB🐖vNQ🐜Ls🐗c🐤🐪🐘2🐷🐁H6🐰🐈🐟🐏i🐖f🐌mlJxI🐳T🐪J

π in base 124: 3.H🐇F🐷🐽JalNda🐏🐉d🐅🐃4Q36🐉🐔s🐀O🐂65🐲Q🐛🐅🐣🐜🐘🐌O3Kw🐗🐛cF🐦F18🐴N1🐀🐓🐅a🐙HZ🐛kidS🐑5🐼tF🐈b🐍c🐪n🐱🐀4E🐳a🐬tl🐡🐔p5D🐔N🐭🐋X🐣🐞🐦🐫🐜🐄C🐰🐭

π in base 125: 3.H🐙mI🐷🐬🐉🐂L🐨🐖umA🐒C3🦀gu5🐩🐘🐥4🐛🐺nT🐆🐮🐎g🐔🐣TpXtu🐰S🐗🐴🐔Ba🐮WtyFA🐪I🐂T🐑8🐸🐸🐌🐼mB5🐄🐂🐵🐋🐂E39🐤🐄Z🐠Q🐍🐜W🐞F🐚m🐯u🐇cySRtQ🐵s🐨cw🐪7

π in base 126: 3.H🐫🐶🐆🐦🐉🐼🐭6M🦀🐒🐽O🐥E🐅kHy🐉🐪2m🐺vG🐓🐊🐐7crSH🐭m🐱ob🐘r🐓🐹🐩q🦀lcT🐳🐕🐃🐃🐓🐲X🐇Nt🐥🐣🐶Ps🐲A🐩🐽V6🐐4🐋🐮FO🐸🐗t🐛Yw🐻🐭z🐢9Qn🐛X🐜Ko🐬dy7Z

π in base 127: 3.H🦀🐠🦁4🐐jLi🐄🦁HC🐩t🐅🐃🐒🐽🐹Zz🐐Tz🐎🐮H🐥6🐅🐊🐉🐡1A🐼f🐵e🐃🐅q🐃🐠e8X🐣🐻i1f🐳🐐🐰🦀🐄M🐳🐩s🦂5🐍🐚🐢🐽🐫T🦂S🐽🐧e🐛🐅🐰🐭🐔gd🐇WN🐩🐹🐴🐮🐅🐦9🐣

π in base 128: 3.IF🐯e🐆M🐈8🐫🐆🐥O🐓u6🐲v🐓K🐂🐋🐢🐆f🐑🐎w0🐃x🐷O🐸J🐏🐊🐌K🐒X🐵EQ1R🐠🦀🐖pp🐨🐐🐊n🐛🐂🐘As🦀🐍🐳X🐟V🐣Q🐝j🐗E9B🐦🐄🐯k🐨I🐻🦁🐈🐼J8k🐏O🐱🐯r🐄🦃🐷🐧🐝

π in base 129: 3.IYVo🐊d🐒Y🐓g🐆🐮🐗🐩🐝L🐷fh🐵🦃N🐇🐎🐶R🐆🐑🐩🐅🐮EB🐙🐐Z🦁mhr🐚🐮T5dO🐩53s🐌HA🐭2k🦂LM🐼🐨nZ🦄🐪C🐔q🐤🐹LN🐘🐽N🐐N🐄nJ🦀t🐛🐅v🐥Bh8u🐁xg🐋Q3S🐈PBU🐘🐨

π in base 130: 3.Iq🐹7🐨🐻C🐅🐘z🦂🐺🐄DW🐨AH🐝0k🐢🐵🐍l🐣DUCjaR🐜🐤u🐭a🐇i3H🐬u6uhkE3e🐒V🐚W🐻🐌🐣xfF🐄🐧🐨QE🦂🐗N🐤PA🐌🦅E🐚🦀🐇Xj4🐔🐬C🐱ywE🦂Y🐊G🐻Y🐀🐶z🐺lHm🐝🐗🐛

π in base 131: 3.I🐉🐴Mc🐮A🐩🐅U🦆u🐫🐛🐪🐻F🐶🐈a🐯🐳0🐘S🐅vd🦆Q🐓V🐬xECzrIQI02r🐓K🐶d🐌R🦁v🐭🐇🐨🐈🐥O🐴Q🐹🦃s🐵🐵🐯🐐🐠🐴🐔🐥🐺mti🐟t🐍J🐩s🐕Qg🐧🐵🦂🐝7M🐨🐼🐪A🦂🐦🐢

π in base 132: 3.I🐝E🐍🐉🐒pe🐋a09🐒🐀B🐆vUr🦄8🐬🐭5w2Qx🐂🐑f🐻🐇🐗🐕T🐵🐕🐫IaN🐛🐦🐀x🐠🐵W🐍n🐯🐫yw🐍🦀O🐳🐲🐤🐂🐜Ii🦁cp🐎🦃🐨t🐦🐱I🐌E🦃🐎🐉🐀🐩D🐅🐞RglLmt🐰🐌naE🦂🐸

π in base 133: 3.I🐰🐖Fq🐤🐍G🐩🐀bXX🐽Y🦅z🐠🐖🐤G🐞BK🐄🐬🐛🦈🐋🐙mz🦄XP🐢XH🐫8K🐠🐥gfJ🐘🐉N3🦂🐫T0🐏IZ🦅🐱🐉s🐪🐝y🐫🐕🐪gz🐗f🐵🐵K6Nf🐇u🐏🐭C🐌🐭🐖i🐹w🐆🐜🐍🦀A🦇3🐀qJ

π in base 134: 3.I🦆w🐙G🐅🐹🐯🐳🐽🐎🐀Yn🐭🐸🐠🦂🐒o🐛K🐒c🐎U🐽mM🐻d🐩🐍Bf🐴R🐠jK🐽VjMt🐶K🦉🐁Q🦅J06🐈DKfGPH3Ukf🐸b🐠eirr🐣🐴🐭fk2🐆🐘n3v🐧M🐅🦇🐝🐂Pi🐣1🐋🐟🐤wc🦇🐏🦅0

π in base 135: 3.JF🐉3q1i🦂4🐲kf🐟🐵🐇🐒S🦅🐭🐒f🐬🐒oBd🐻🐩🐔ZU🐤7🐛2it🐱d🐴T6R4i🐢34Z🐫7🐝Ya🦁z🐋🦅2🐽🐐🐕N🐓🐀🐀4🐊VG🐗A7🐛c🐼🐺🐽gr🐯P🐦🐺🐉🦂C7🐖6🐘🐛🐵P🐩🐼5🐢🐩🐯

π in base 136: 3.JY🐼CXMr🐲🐋🐥🐏🐮Ap🐵N3🐽🐬🐩🦇3🐔🐂🦀n🐁x🐘🐋🐽H🐊🐛🐪V🐞🐙d🐒O🐯3🐠🦃ogZBcp🐟B🦉nHaGk🐹🦊🐍🦃I🐝nv🐱🐀🐭1🐻2ptM🐵🐽A🐔🐧🐮FyU5g🐻🐣CX🐅🐮🐅🐻D🐋🐝A

π in base 137: 3.Js🐍🐡L🐟🦋🐊🦀🐀i🐅g🦂q🐷🐛🐟B🐱fBA🐯🦊B🐸D🦊🐩🐹Xp🐩c🐍🐽🦆🐵s🐛🦋wL🦋🐰🐲d🐶V🐏hqdZ🦈🐎🐽p🐆Ps🐐🐰V🐧🐲w🐚🐝🐇3JByvv🐯🐣🐴🐤🐙🐙🐗🦂🐃🐒mY🐪🐉🦋f🦂

π in base 138: 3.J🐌🐅🐠🦌BQY8W🐴m🐼🐛🐋34🐡🦊H🐘9🐖🐪2🐗🐜K5l🦋H🐇🐬🐃🐉🦃Y3f🐝I3🐷🐻u🐳🦅1Q🐝🦍🐱🦋U🦈🐁🐣y8🐭🦊🐭🦇40🐪HS🐽O🐤🐕🐖🦊🐗🐶🦉P🐂6🐜ZC🦁🐉🐙d🐐🐫Wm🐚Q🐫

π in base 139: 3.J🐠🐤🦃🦌🐚🦋E🐞🐒OU🐡k🦆🐷f🐜5🐍UW🦃🐉kMcTX🦅K🐓s🐋Olj🐋Q🐰sL🐫W4🐕🐗🐍9🐅i🐜🐣v🐤🦉🐅🐟🦂🐁G🐠9🐃oHp🐫🐛🦈🐷🦈🐄🐆uJ🐦🐯6o🐊yU🐘🐇🐫🐐C🦇🐄wQDs🐅IA🐒

π in base 140: 3.J🐵UX🐲x🐼🐷🐂🐬🦁🐀🐅n🐢🐐🐰🐛H🦈🐶Iz🐬🐏🐉🐈🐢🐲🐣🐢🐼🐝🐋🐀W🐂🐃🦈CV🐲🦁🐰🐆axA🐵5j🦃p🐑🦃🐐🦎c🐍🐃🐚🦄🐅G🐷Ep🐋O🦊🐎🐙🐞0🐸🐏🐍🦋🐋6🦇lk🐬🐋EW🐱Ai

π in base 141: 3.J🦌0🐈🐈c🐛🦋S🐊🐯s🐵🐮m🐔qb🐗🦂🐟🦀L🐡re🐃🐔7🐘🐳🐇Ap🐶lQc🐹VA2z🐆🐞🐑6🐨🐌R7🐈🦇🐔🐃a9🦂ZZK🐆🦏🐒🐢🐐🐻L🐒🦐🦆🐂🐨🐓v🐳🐱🐦I🦇🐶🦐🐲Q🐟🐃i🐶🐣🐔D🐰🐔

π in base 142: 3.KFA🐏🐋🐮🐫C🦇🐨🐬s🦐U🐺🐉🦇🐐🐩🐭Kr🦈JG🐰E🐸🦑🐯Ec🐨e🐏9kAr🐒🐯🐼🐒e🐺L🐴V🦀Kx🐶e🐗7n🐕ul🦎🐉s9zL🐛🦇🐑🐂iQ🐞v🦁🐑a2p🐏🐉🐫y🐱🦁fu🐩1whAu🐺🐳🐏🐁i🦈🐜

π in base 143: 3.KZzW🐨X🦊CcCRI🐀s4R6🐠🐫🐫🐴g🐴🦁4🐭Ig🐫nu🦄🦆O4🐹🐇🐄🐫🐀9🐂🐻🦐🐐🦎🐺🐸🐮🦀🐷🦒🐄JN8🐧🦃y🦀🐀uZ🐮L3CD52🦋🐤🐫🐯🐎🐠07🐒🦉🦈🦐🐩o🐳kr🐃🐄🐨🐧🐫🐷L🐐🐼

π in base 144: 3.Ku9vl🐯🐨🐎🐅k🐌D🦓HHz🐺🐅XXA🐞9🦀🐰🐚Pm🦂A🐃T🐔6🐋🐺h🐁🐽🦆🐂🐢🐄s🐪1🐣🐽🐒a🐥VZ🦓🐺🐽🐖🦉Q🐏🐀🐸🐤🐫3🦌🦊🐄🦐mh🐇🦃IU🦐🦁s🐩🐌Kv🦐v🦄Q🐄I🦌o🐟🐔b🐙🐡

π in base 145: 3.K🐎🦒🦇2K🐤g🦈🐥🐀1vr🐱🐍🐞F🐄🦓rY🦂🐠wz🦀🐠🐛🦔🐘🐦🐢🐴🐙🐐4🦂u🐤🐔5🦐🦄🐘🐶🐄🐟🐦🐀k🐟e🐐🐚🦌1R🐄🦊y🦂I🐳🐑k🐇🐎🐝🐜🐅🐔k🐘b🐐🦋u🐰🐹🐌🐌🐫X🐂🐴VTrf

π in base 146: 3.K🐤R🐘🐷🦇🦌🐬🦂🐠🐚i🦍LZ🐁x🐲🐤🐪🐐🐦🐄E🦊cj🐯🐛🐋p41🐰8🦋🐟t🐪🐺🐴🐖r🐷🐬🐈X🐂zz🐽vA2c🐹🐈Zv🐏🐐🐮🐊m🐇A🦃🦏🐹W🐰🐊X🦊🐬wE🦋🐅🐹3🐌🐥🦄🐌🐖B🦀🦊🦊v🐚

π in base 147: 3.K🐹🐥lMftNS4🐢🦔🐌i🐢🐜🐚🐟C🐸T🦆Ur🦏xX🦈🐐r🐡1🐟🐽🦁🐂🐊🦅🐐🐴B🐍🦌🐢🐋🐥🐆TuU1🐵🐔🐀t5🐌a🐂🦈🦓🐈RP🐸E🐱S🐓vJ🐔7🐻o🐻🐂e🦌🦐jLL🐎🐑L🐇eti🐒x7🐮🦖🦉🐂

π in base 148: 3.K🦑🐃🦍🦇V🦋b🦄aXE🦆🐪🐇🐮H🦈🐊szJ5qtD🦓🐵🐞🦎wKdU🐲FMGDOE🦌🐜Hl🐛e🐖🐡D🐁🦓🐴🦍Gv🐻🦓🦖🐰🐪🦒e🐍🐒LS🐪v🐘jk5🐰🐐🦋🐂u🐋R🐶B🐛🐂🐊Gi🐄🐹F🐎🦌🐁pme🐔O🐊

π in base 149: 3.LE🐌fbo🐪1A🦐q🐮l🐩Y6🦂🦎🐲t🐏lA🦕weG🦐🐇🦀bu🐯z🐮🦖🦐jvIPO9🐏i🐭1🐆i🐮M69🦇🦕h🐧🐹p🐮🐢H🐙🐱🐯🐂4🐸sSqYpQyGEn🐓dz🦔O🐈🦗X🐩V🐘🦍6🦈🐵🦐K🐻🐤🦈X🐈🐃F🐕

π in base 150: 3.LZ🦁U🦇🦖Z🐸🐤O🐦🐟m🐒🦆🐹cT🐱🐄🐽🐧4bodeu🦏AR🦉🐬🐋🦎🐇🐛G🐍🐠🐽U🐴🦇🐏🐃🐴K🐧TA🦂🐻🐱🐽C🐢🐞🦁vF🐽xaB🐴zFbgY🐹🐛🐖Q6R🐠🦑🐓🐱🐶🐘fBU🦙🐰nDt🦁🐸f🐫🐀

π in base 151: 3.Lv🐆🐗🐼H🐍f🐺🦍5🐭U🦇🐚An🦕🦄mN🐰🐌🐛Y🐕🐇🐔🐼1🐍1jT🐷4🐔🐧🦃cV🐃🐁🐑3🐧🦆🦅Bk🐅4🦂🦔🐘🐸🐇h🐯🐕🐌🐅g1🦎🐙SM🦐🦑8vNf🦆🐛y9🐰🐆zGa🐲🐐🐺🦔I8Y🦃21E🦗A🐦

π in base 152: 3.L🐑sW🐉🐂🦐🐱🐻F🐦k🦍4wc🦅3🐉🦂w🐹J🦃CH🐇🐢S🐢🐮🐌z🐔🐞🐧🦚y🦙🐲58🦒5🐐🐮🐙🦕🐍n🐷MRd🦒🐮h🐣🐔t0🦈🐤🐴N🐲mdq🐫3🦌Ct🦚🐋Y🐇🐘p🐅GECj🐲🦂🦙🦑🐁🐏🦃🐀🐆9

π in base 153: 3.L🐧🐔🦛🐬Z🐄y🐫R🐟pFK🐈🐜🐋m🦒R🦃🐋3🦖🦅🐯🐶iJ🐄KZr🐠zv🦈3🐋🐂🦎4🐪🦎🐱🦚🐋🦛O🐃Cxf🐃🐞🐔🦒🦄D🦛🦐🐦🐷🐬02🦆🦈🦙🐸🦂w🦊q🐘🦁D🦎xd🐉🦌🐬🦉y🦚dE🐂🐗🐼🦋

π in base 154: 3.L🦀1🐵🐯🐏o🐋🐴🐬🐁🐹🐣UFMm🦅Q🦘🦙YL🐷🐵🐺🦛X🦃🐬🐀WID48W🐩Z🦆Kik🐹🦓4kN🐛u4wBIk🐋D🐞🐫🐣🐤🦄🐎🐴🦍5n🦔🐛🐈🐃🦇e🐰Ziq🐍🦀MsCkJ🐋S🐹🦌🐲🦝🦀k🦈🐹🦆🦚🐐

π in base 155: 3.L🦖🐸rM🦈R🐒🐢🦕🦉🦑🐟ZW5🐱🦕xc🐓🐒140🐐8vUc🐽🦃N🐬🦇🦞n3i🐭L🐤🐃g🦌pJ🐙r🐜🐀🐞S4🐛M🐪0🐻F2🐞🐚a🐘g🐈🦜🐽Q🦂I🦛🐁🐚H🐢🐱🦀SN🐧🐅🐪9🦑🦀🐑Z🐘🐈🐇🦛p🦌k

π in base 156: 3.MD🦀🐢4🐔🐥🐥KTE🦁🦔🐆🐑b🐓🐅🦟🐆🐧u🐥🐼🦞🐄🐧🐀c🐗🦄🦂i🐯🐸y🐯3D🐈w🐭H🐳🐲5🦞A🦍🐞🐘KtgQ🐍P🦉8🐘🦗🦃🐝🐵w🐐🐤s🐀🐓W🦚🦛x🐄0🐕🐂MPZ🦏IQrf🦈X🐢1🦒kt🐌🐐

π in base 157: 3.MaI🐃🐼🐁Yp🐍🐭🐟Q🐩Bj🦎zq🐬W🐅🐕🦅🐶dR🐟🐕uusl🐢G5QwIK🐕🦊🦆xQa🐂🐟🐠🦘🐑🦚EFO🐠h🦑🐁🐏CJ🐕🦎🦂A🦙g🐉z🐪🐞6x🐟1D🐊🐪🦠🐔pT🐀T🐟🦖Z🐸🐴🦅🦙G🦜🐞🦘q🦂J

π in base 158: 3.Mw🐳🐡Y🦚🦠🐎C🦁🐠🐴🐧🐬🐣🦐pER5AL9🦅C7s🐕eF🦁2🐏🐈🦝b🐣🦠🐋G🐕T🐒🦆hi🐰🐶🦘PB6🐃🐌🐀🐖66🐃🐠T🐃🦊M🐁🦋🦋🦍🦙🐔f🐐N🦄h🦅N🐇🦏Co🦁🐎🐵R🐖🦗🦛🦊🦄B🐙N🐲

π in base 159: 3.M🐓🐢2🐝ieP🐫q🐼🐠🐁25H8🐦🐊🐬KFG🐵🦍🐝i🦈🐥🐑J🐘🐂Ie7🐜🐤🐚🦂🐝🐮TR🐑Y🐊🦁Kl🐤🦏🦟k🦏🐇🐎🐵🦇🦞🦝🦠q🐑🦟🐳🦒🦖mB🐤🐉c🦜ND🐙🦏7🐚🐽k🐙🦘v🐠QG🦇sR🦛🐹🦖

π in base 160: 3.M🐪🐽🐓🐋9L🦋🦍🐟🐁k🦐🐐🐌🦚🦣🐒T🐇🐦🐹🦄🐋🦔y🐞🐷🐽🐖🐉🐼🐎🐋6o🐱t53🐴🐊🐁🐔k🦋🐊Em🐹🦀u🐟🐚🦘E🐽w🦘K🐩🐄🐁l🐨bj4🐸9🐼🦈🐅y🐵k🦘Lu🐷🦒🦞l🦒🦛🐨U🐋Yf🐸

π in base 161: 3.M🦄Z🦙🦒🐝P🐹M🐯l🦄🐊🦆i🦆🐥2🦗h🐣🐍🦣🦈d🦢🐥E🐊U🐱s🐕🐖🦍🐓🐉v30🦠c🐢T🐋🦚🦁D🐬a4J🐔🐑🐱🐈🐢🐖🦅QfEHO🐟🦎ZY8🐼🐛🐁🐸🦆🐒🐝🐏D🦔🦕i🐂B🦘🐙🐗🦓🦠aaaX🦂

π in base 162: 3.M🦛🦟Li🐬🐷🐸K🐏nm🐔🐴🐭🐷🐋W🦝🐅S🐢🐛🦉4🦢🦚x🐐🐫🦥🦝🐢🐞🐷DA🦟🦂ia🐋🦓🐇🐧🦆🐼🐵🐀🐦tiq🐢🦑q🦄🐏iRW🐱🦥e🐹Kiv🐅🦋6vKy🐉🐋🐌🐝🦣🦈🐢🦥🐹Q🐰🐨🐲🦔🐗yZ

π in base 163: 3.NC🦢🦠🐉🐽🐟🐶o🐻Qg🐍🐔🐛u🐜🐈w🐡🐍🐃🐏🦁🐎GP🦈🦆0p🦢🐳r🐥p🐖🐦🐞🦛🦚🐠P🦝a🦣🦣nq🐧🦉t🐽🐈🐪y🐫🦛XlR🦍y🐳🐌🐃🐏c🦞Tsr🦘🦋OAG🐇s1🦤8🦉qp🐥🐹🦣O🦢🐇A🐶🦖

π in base 164: 3.Najh🐢🦏R🦍aH🐌🦛VrN🦋🐦🐋🦎🦥🦅S🦆🐐🐻🦂iL🦋🦐🐹🐅9🐠v🐚🐈🦍s🦇kK🦡XRw🦞🐏🐨🐰🐷JQ🐤🐊🐃🦆k🐉🐎5o🐏🐥🦈🐰🐇🦌🦏I🦗🦠🐵🐜🦆🐳jK9🐏🐬🐹J🐒🐤🐷🐵B🦠X🦦U

π in base 165: 3.Nx🦑🦘t🐏7o8W1XX🦌🐥gC🦋🐴🐑🐙0V🦎🐛🦋cFa🐌🐔🦘🦗d🦋🦊🐭a🦚🦅🦁🦎kTN1qI🐫🐷2🦔🦟🦚k1🦤🦇K🐌🐸🐳🦨v🐖g🦀🐜🦗🦔🐂HTF🐠🦅x7Ng🐫🦘z🐋a🐔zW🐳7🦧l7q🦤🐍🐻N🐧

π in base 166: 3.N🐕🐺🐷🐳🐵E🐂🦓w🐓J🐽🐷🦐🦥iE6🐕ak🐨🐪xL🐊🦈🐹🐐Toq🦦🐃🐬mYF🐚🐓o🦈eGoO🐎🐭🦞o🦇I🐖🐶🐀LB🦞cM🐽🐙3🦒🐷Y🐶🐜🦎🐺🐐IVb🐯L🐷🐋🐲🦆🐴s🐘🦊n🐊g🐑6🐳🐙OT🦒

π in base 167: 3.N🐭🦖🐝7N🦅l🐈🐛🐓ufXG🐖🦨🦊🐥🦧🦔k🐴j🐵🐲🐹🦊🦑🐒q🐚🦋🦈q🐉c🦃L🐲X🦃🐥h🐱d🐤🐚🦚🦂🐷F🐩4Y🐯🦙D🦥n🐗🐑🐯🐈🦤M🐐x🦝Y🦇DH🐚s🦇🐯🐊J4W🐵🦂🦒0🐟🐧🦜Y🐤🐦🦁

π in base 168: 3.N🦈qhZ🦤xy🦁g🦕🐢lMiQX🐍🐄🐖🦑🦒3🐆🐖🐻🐨🦌🐧🦄🦐q🦒UR🦀🐉🐏🐣🐥F🦠🐷jVSWuIvTT🐗🦧🐻🐑🦊🦑🐷🦪3🦙z🦂🐵X🐒🦙P🐏S🐓🐙🐫🐆🐊Ui🦨🐂Z🐢🦄🦙🦦YE🐥🐙🐺bv🦩🐠

π in base 169: 3.N🦡4🐷🐅🦈🦧🐠🐙🦠🦞🐳🦬🦁🐭X🐬🦁i🐵k1🐐d🐑g🦪🐍🦫🦋🐣H🦙🐎r0Q🦢b🐂🐞v🐧🐼🐍🐄🐈🦓🐀🐜🦙🦬czyt🦎🦞🦞🦏🦆m🦚🐨h🦫C3🐜🦒🐣EZ🦖J🐵🐵🦑🐰1h🦤M🐥🐩🐸LF🦁🐉

π in base 170: 3.OC4🐺Y🦌🦀🐌🐔🐣🐙🐽R🐜🦗🐳🐺vx2🦐4O🐴🐭🐲🦤🦂🦪🦬🐕🦁🐣🐆🦁xj🦟🐗🐮🐀🐦🦋🦏9🐩🦧🐲🐌🦫e🐔🐗ox🐩🐬🐀🐴🦞L🐳🐖L🦄🦀🦕t🐽🐯🐨🦭🦇4🐸u🦅🐇🐢🦉🦝q🦊🦣🦊W🦔
"""
