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
