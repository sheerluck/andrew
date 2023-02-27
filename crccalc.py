import itertools
from fastcrc import crc16, crc32, crc64


def radix(val, base):
    res = []
    while True:
        head, tail = divmod(val, base)
        res.append(tail)
        if head < base:
            if head > 0:
                res.append(head)
            return res
        val = head


def int2str(x: int) -> str:
    base = 20_180  # 40_940 - 20_760
    shift = 20_760
    bits = []
    for mod in radix(x, base):
        bits.append(chr(shift + mod))
    return "".join(reversed(bits))


def int2bytes(x: int) -> bytes:
    if 0 == x:
        return b"\x00"
    return x.to_bytes((x.bit_length() + 7) // 8)


data = b"1"
crc = []
for mod in ["crc16", "crc32", "crc64"]:
    alg = eval(f"{mod}.algorithms_available")
    fun = [(f"{mod}.{f}", eval(f"{mod}.{f}")) for f in alg]
    crc.append(fun)
    for name, f in sorted(fun):
        print(f"{name:<25}: f({data}) = {f(data)}")

fun = [f for (n, f) in sorted(itertools.chain.from_iterable(crc))]
hsh = [str(f(data)) for f in fun]
line = "".join(hsh)
i = int(line)
name = "crc.combined"
print(f"{name:<25}: f({data}) = {int2str(i)}")

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
crc.combined             : f(b'1') = 謡鍫窴磷澐漟算尦屩泟廷玁隚胪驏籈潅緪嗌鬨舉儻沿猈陿郣啋犍覐唫衊濔彼汕苇笠瘍鱤敡枻曗妙巚踵覌橖熥皏蟁彶飱搳睵懓塑闦述謇髌绤荗磋牌蓼芹飶曘瑌癪酫肰衈莞鉌辕
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
        val = f(data).hexdigest(suf // 8)
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

from portage.util.whirlpool import CWhirlpool

name, f = "Whirlpool", lambda x: CWhirlpool(x).hexdigest()
print(f"{name:<25}: f({data}) = {f(data)}")

"""
Whirlpool                : f(b'1') = 8513c69d070a008df008aef8624ed24afc81b170d242faf5fafe853d4fe9bf8aa7badfb0fd045d7b350b19fbf8ef6b2a51f17a07a1f6819abc9ba5ce43324244
"""




# https://www.integers.co/questions-answers/what-are-the-different-hash-algorithm-outputs-for-1.html
"""

md2                                  c92c0babdc764d8674bcea14a55d867d
md4                                  8be1ec697b14ad3a53b371436120641d
✅md5                                c4ca4238a0b923820dcc509a6f75849b
✅sha1                               356a192b7913b04c54574d18c28d46e6395428ab
✅sha224                             e25388fde8290dc286a6164fa2d97e551b53498dcbf7bc378eb1f178
✅sha256                             6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b
✅sha384                             47f05d367b0c32e438fb63e6cf4a5f35c2aa2f90dc7543f8a41a0f95ce8a40a313ab5cf36134a2068c4c969cb50db776
sha512/224                           a5d8adf6032b5f333d9cd6696dd0c520b4dca7f0c3238aa8de33e87c
sha512/256                           18d27566bd1ac66b2332d8c54ad43f7bb22079c906d05f491f3f07a28d5c6990
✅sha512                             4dff4ea340f0a823f15d3f4f01ab62eae0e5da579ccb851f8db9dfe84c58b2b37b89903a740e1ee172da793a6e79d560e5f7f9bd058a12a280433ed6fa46510a
✅sha3-224                           300d01f3a910045fefa16d6a149f38167b2503dbc37c1b24fd6f751e
✅sha3-256                           67b176705b46206614219f47a05aee7ae6a3edbe850bbbe214c536b989aea4d2
✅sha3-384                           f39de487a8aed2d19069ed7a7bcfc274e9f026bba97c8f059be6a2e5eed051d7ee437b93d80aa6163bf8039543b612dd
✅sha3-512                           ca2c70bc13298c5109ee0cb342d014906e6365249005fd4beee6f01aee44edb531231e98b50bf6810de6cf687882b09320fdd5f6375d1f2debd966fbf8d03efa
ripemd128                            964297086cacedf34c500708065bca73
ripemd160                            c47907abd2a80492ca9388b05c0e382518ff3960
ripemd256                            03f0670dd2cd5186bc159a0bdd40207c6044b9fca7fb2b72ce1ab8713d3ce9b1
ripemd320                            0cd35e506b546c0327a52783b6dac40c766e0bc583fda558438b92816e9be0fe7e0aeff65b07439e
✅whirlpool                          8513c69d070a008df008aef8624ed24afc81b170d242faf5fafe853d4fe9bf8aa7badfb0fd045d7b350b19fbf8ef6b2a51f17a07a1f6819abc9ba5ce43324244
tiger128,3                           1d573194a056eb3200f9d302900c843c
tiger160,3                           1d573194a056eb3200f9d302900c843c3d41ab4e
tiger192,3                           1d573194a056eb3200f9d302900c843c3d41ab4ed06c03df
tiger128,4                           f3f1eb5f84be12b1491594425324d529
tiger160,4                           f3f1eb5f84be12b1491594425324d5299f3abb37
tiger192,4                           f3f1eb5f84be12b1491594425324d5299f3abb37c49e9c01
snefru                               2f9c342624a2574411cbef831287e210f1da98223f30a5e34cdaa61430f659fa
snefru256                            2f9c342624a2574411cbef831287e210f1da98223f30a5e34cdaa61430f659fa
gost                                 b0f784fe99f37c57188d100f79bffa0e877f38c8ad50baf7e474b7596a02b5bf
gost-crypto                          5ee4b6353be6190473db5d56d2b561c961d0748a74cd55c419e7af1557d126ab
adler32                              00320032
❌crc32                              6c300461    as crc32.bzip2    = 61-04-30-6c   // reverted 6c-30-04-61 
✅crc32b                             83dcefb7    as crc32.iso_hdlc = 2212294583
fnv132                               050c5d2e
fnv1a32                              340ca71c
fnv164                               af63bd4c8601b7ee
fnv1a64                              af63ac4c86019afc
joaat                                806b80c9
haval128,3                           e741857e1406b389f8b78284bc07cd88
haval160,3                           0bd4fe48c20d972bfa052b37c0ce8751aaafc6e0
haval192,3                           64f8a98d5c6d887cfa2d8e0ca75be56ee426fb60a82aa402
haval224,3                           797e64a16406e4f634189051033f23af2ea9b08639a1ed26b266e2b0
haval256,3                           160169a53ff6af085a928075e8328f3f1939544ffafab649acaeec68f7c38a06
haval128,4                           2a3d6347b805a0e458de7bb9a59b1bcf
haval160,4                           c1f44f4d32ce6b10fcb6ec71f292cfa43323ee6c
haval192,4                           5c46ca61059c10a4f704ad80e5ae84e7306d631e522ed887
haval224,4                           07f9304605773b701236d4ce1552cd0d27f151cbe34fcf0797792ee2
haval256,4                           b989c5637fa6e10641c18fd913f24c83ad1cd0f558a15b2981b985e4efdaab0f
haval128,5                           50034bd20ef09138e2ff633365ea6879
haval160,5                           79a1230ab53892519541f6ce0ccc0b5c1ec2f1ed
haval192,5                           2fcfad0fad45df44302adeef37670fbad3fd08f61e55b57c
haval224,5                           4c0eb619d17c8633d92bf3f91ed5ad4c6e63e491fc8f8d48b1d982a0
haval256,5                           7045e5094aee9618214c1218b575d7f2325645e2830622a706ec39616e16a936
"""
