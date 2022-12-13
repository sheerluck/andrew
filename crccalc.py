from fastcrc import crc16, crc32, crc64

data = b"1"
for mod in ["crc16", "crc32", "crc64"]:
    alg = eval(f"{mod}.algorithms_available")
    fun = [(f"{mod}.{f}", eval(f"{mod}.{f}")) for f in alg]
    for name, f in sorted(fun):
        print(f"{name:<25}: f({data}) = {f(data)}")

"""
crc16.arc                : f(b'1') = 54465
crc16.cdma2000           : f(b'1') = 52559
crc16.cms                : f(b'1') = 32167
crc16.dds_110            : f(b'1') = 3750
crc16.dect_r             : f(b'1') = 60472
crc16.dect_x             : f(b'1') = 60473
crc16.dnp                : f(b'1') = 21093
crc16.en_13757           : f(b'1') = 19966
crc16.genibus            : f(b'1') = 14461
crc16.gsm                : f(b'1') = 55693
crc16.ibm_3740           : f(b'1') = 51074
crc16.ibm_sdlc           : f(b'1') = 53362
crc16.iso_iec_14443_3_a  : f(b'1') = 29172
crc16.kermit             : f(b'1') = 8202
crc16.lj1200             : f(b'1') = 3627
crc16.maxim_dow          : f(b'1') = 11070
crc16.mcrf4xx            : f(b'1') = 12173
crc16.modbus             : f(b'1') = 38014
crc16.nrsc_5             : f(b'1') = 2110
crc16.opensafety_a       : f(b'1') = 6284
crc16.opensafety_b       : f(b'1') = 52197
crc16.profibus           : f(b'1') = 50642
crc16.riello             : f(b'1') = 47550
crc16.spi_fujitsu        : f(b'1') = 60142
crc16.t10_dif            : f(b'1') = 30432
crc16.teledisk           : f(b'1') = 53650
crc16.tms37157           : f(b'1') = 42301
crc16.umts               : f(b'1') = 32933
crc16.usb                : f(b'1') = 27521
crc16.xmodem             : f(b'1') = 9842
crc32.aixm               : f(b'1') = 2842260683
crc32.autosar            : f(b'1') = 770158430
crc32.base91_d           : f(b'1') = 2602376699
crc32.bzip2              : f(b'1') = 1627664492
crc32.cd_rom_edc         : f(b'1') = 2341548289
crc32.cksum              : f(b'1') = 789352408
crc32.iscsi              : f(b'1') = 2432014819
crc32.iso_hdlc           : f(b'1') = 2212294583
crc32.jamcrc             : f(b'1') = 2082672712
crc32.mpeg_2             : f(b'1') = 2667302803
crc32.xfer               : f(b'1') = 8127
crc64.ecma_182           : f(b'1') = 16768987096479742114
crc64.go_iso             : f(b'1') = 4836865999795912704
crc64.we                 : f(b'1') = 8235833358291897690
crc64.xz                 : f(b'1') = 3039664240384658157
"""

def sort(p):
    if "xxh3_" in p[0]:
        key = p[0] + "*" * 100
    else:
        key = str(p[1](data))
    return len(key)


import xxhash

alg = xxhash.algorithms_available
fun = [(f"xxhash.{f}", eval(f"xxhash.{f}_intdigest")) for f in alg]
for name, f in sorted(fun, key=sort):
    print(f"{name:<25}: f({data}) = {f(data)}")

"""
xxhash.xxh32             : f(b'1') = 3068971186
xxhash.xxh64             : f(b'1') = 13237225503670494420
xxhash.xxh128            : f(b'1') = 296734076633237196744344171427223105880
xxhash.xxh3_64           : f(b'1') = 7335560060985733464
xxhash.xxh3_128          : f(b'1') = 296734076633237196744344171427223105880
"""

def sort(p):
    name, f = p
    try:
        key = f(data).hexdigest()
    except Exception:
        suf = int(name[-3:])
        key = "k" * 32 + "*" * (suf // 100)
    if "sha3_" in name:
        key += "*" * 100
    if "blake" in name:
        key += "*" * 200
    return len(key)


import hashlib

alg = hashlib.algorithms_guaranteed
fun = [(f"hashlib.{f}", eval(f"hashlib.{f}")) for f in alg]
for name, f in sorted(fun, key=sort):
    try:
        val = f(data).hexdigest()
    except Exception:
        suf = int(name[-3:])
        val = f(data).hexdigest(suf//8)
    print(f"{name:<25}: f({data}) = {val}")

"""
hashlib.md5              : f(b'1') = c4ca4238a0b923820dcc509a6f75849b
hashlib.shake_128        : f(b'1') = ebaf5ccd6f37291d34bade1bbff539e7
hashlib.shake_256        : f(b'1') = 2f169f9b4e6a1024752209cd5410ebb84959eee0ac73c29a04c23bd524c12f81
hashlib.sha1             : f(b'1') = 356a192b7913b04c54574d18c28d46e6395428ab
hashlib.sha224           : f(b'1') = e25388fde8290dc286a6164fa2d97e551b53498dcbf7bc378eb1f178
hashlib.sha256           : f(b'1') = 6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b
hashlib.sha384           : f(b'1') = 47f05d367b0c32e438fb63e6cf4a5f35c2aa2f90dc7543f8a41a0f95ce8a40a313ab5cf36134a2068c4c969cb50db776
hashlib.sha512           : f(b'1') = 4dff4ea340f0a823f15d3f4f01ab62eae0e5da579ccb851f8db9dfe84c58b2b37b89903a740e1ee172da793a6e79d560e5f7f9bd058a12a280433ed6fa46510a
hashlib.sha3_224         : f(b'1') = 300d01f3a910045fefa16d6a149f38167b2503dbc37c1b24fd6f751e
hashlib.sha3_256         : f(b'1') = 67b176705b46206614219f47a05aee7ae6a3edbe850bbbe214c536b989aea4d2
hashlib.sha3_384         : f(b'1') = f39de487a8aed2d19069ed7a7bcfc274e9f026bba97c8f059be6a2e5eed051d7ee437b93d80aa6163bf8039543b612dd
hashlib.sha3_512         : f(b'1') = ca2c70bc13298c5109ee0cb342d014906e6365249005fd4beee6f01aee44edb531231e98b50bf6810de6cf687882b09320fdd5f6375d1f2debd966fbf8d03efa
hashlib.blake2s          : f(b'1') = 625851e3876e6e6da405c95ac24687ce4bb2cdd8fbd8459278f6f0ce803e13ee
hashlib.blake2b          : f(b'1') = 1ced8f5be2db23a6513eba4d819c73806424748a7bc6fa0d792cc1c7d1775a9778e894aa91413f6eb79ad5ae2f871eafcc78797e4c82af6d1cbfb1a294a10d10
"""

from blake3 import blake3

def blake3_32(b):
    return blake3(b).hexdigest(32)

def blake3_64(b):
    return blake3(b).hexdigest(64)

fun = [(f"blake3.blake3_{8 * n}", eval(f"blake3_{n}")) for n in [32, 64]]

for name, f in fun:
    print(f"{name:<25}: f({data}) = {f(data)}")

"""
blake3.blake3_256        : f(b'1') = d63bd9a826af91c1fea371965a64e11ee20f13e46b5f52c59901136605b3a487
blake3.blake3_512        : f(b'1') = d63bd9a826af91c1fea371965a64e11ee20f13e46b5f52c59901136605b3a487dfd5f9d0daf8dadaeabb1e57f6fc57c057579a87814ccb65d2bca30df7ca55a9
"""
