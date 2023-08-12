/*
 * BMW implementation.
 * CubeHash implementation.
 * Fugue implementation.
 * Keccak implementation.
 * Luffa implementation.
 * Shabal implementation.
 * SHAvite-3 implementation.
 * SIMD implementation.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2007-2010  Projet RNRT SAPHIR
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@cryptolog.com>
 */

#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <iostream> // DEBUG

typedef unsigned int sph_u32;
typedef int sph_s32;
#define SPH_C32(x)    ((sph_u32)(x ## U))
typedef unsigned long sph_u64;
typedef long sph_s64;
#define SPH_C64(x)    ((sph_u64)(x ## UL))
#define SPH_64  1
#define SPH_64_TRUE   1
#define SPH_T32(x)         ((x) & SPH_C32(0xFFFFFFFF))
#define SPH_ROTL32(x, n)   SPH_T32(((x) << (n)) | ((x) >> (32 - (n))))
#define SPH_ROTR32(x, n)   SPH_ROTL32(x, (32 - (n)))
#define SPH_T64(x)         ((x) & SPH_C64(0xFFFFFFFFFFFFFFFF))
#define SPH_ROTL64(x, n)   SPH_T64(((x) << (n)) | ((x) >> (64 - (n))))
#define SPH_ROTR64(x, n)   SPH_ROTL64(x, (64 - (n)))
#define SPH_INLINE inline
#define SPH_DETECT_UNALIGNED         1
#define SPH_DETECT_LITTLE_ENDIAN     1
#define SPH_DETECT_UPTR              sph_u64
#define SPH_DETECT_AMD64_GCC         1
#define SPH_UNALIGNED         SPH_DETECT_UNALIGNED
#define SPH_UPTR              SPH_DETECT_UPTR
#define SPH_LITTLE_ENDIAN     SPH_DETECT_LITTLE_ENDIAN
#define SPH_AMD64_GCC         SPH_DETECT_AMD64_GCC
#define SPH_LITTLE_FAST              1

static SPH_INLINE sph_u32
sph_bswap32(sph_u32 x)
{
    __asm__ __volatile__ ("bswapl %0" : "=r" (x) : "0" (x));
    return x;
}


static SPH_INLINE sph_u64
sph_bswap64(sph_u64 x)
{
    __asm__ __volatile__ ("bswapq %0" : "=r" (x) : "0" (x));
    return x;
}

static SPH_INLINE void
sph_enc16be(void *dst, unsigned val)
{
    ((unsigned char *)dst)[0] = (val >> 8);
    ((unsigned char *)dst)[1] = val;
}

static SPH_INLINE unsigned
sph_dec16be(const void *src)
{
    return ((unsigned)(((const unsigned char *)src)[0]) << 8)
          | (unsigned)(((const unsigned char *)src)[1]);
}

static SPH_INLINE void
sph_enc16le(void *dst, unsigned val)
{
    ((unsigned char *)dst)[0] = val;
    ((unsigned char *)dst)[1] = val >> 8;
}

static SPH_INLINE unsigned
sph_dec16le(const void *src)
{
    return (unsigned)(((const unsigned char *)src)[0])
        | ((unsigned)(((const unsigned char *)src)[1]) << 8);
}

static SPH_INLINE void
sph_enc32be(void *dst, sph_u32 val)
{
    val = sph_bswap32(val);
    *(sph_u32 *)dst = val;
}


static SPH_INLINE void
sph_enc32be_aligned(void *dst, sph_u32 val) { *(sph_u32 *)dst = sph_bswap32(val); }

static SPH_INLINE sph_u32
sph_dec32be(const void *src)                { return sph_bswap32(*(const sph_u32 *)src); }

static SPH_INLINE sph_u32
sph_dec32be_aligned(const void *src)        { return sph_bswap32(*(const sph_u32 *)src); }

static SPH_INLINE void
sph_enc32le(void *dst, sph_u32 val)         { *(sph_u32 *)dst = val; }

static SPH_INLINE void
sph_enc32le_aligned(void *dst, sph_u32 val) { *(sph_u32 *)dst = val; }

static SPH_INLINE sph_u32
sph_dec32le(const void *src)                { return *(const sph_u32 *)src; }

static SPH_INLINE sph_u32
sph_dec32le_aligned(const void *src)        { return *(const sph_u32 *)src; }


static SPH_INLINE void
sph_enc64be(void *dst, sph_u64 val)
{
    val = sph_bswap64(val);
    *(sph_u64 *)dst = val;
}

static SPH_INLINE void
sph_enc64be_aligned(void *dst, sph_u64 val) { *(sph_u64 *)dst = sph_bswap64(val); }

static SPH_INLINE sph_u64
sph_dec64be(const void *src)                { return sph_bswap64(*(const sph_u64 *)src); }

static SPH_INLINE sph_u64
sph_dec64be_aligned(const void *src)        { return sph_bswap64(*(const sph_u64 *)src); }

static SPH_INLINE void
sph_enc64le(void *dst, sph_u64 val)         { *(sph_u64 *)dst = val; }

static SPH_INLINE void
sph_enc64le_aligned(void *dst, sph_u64 val) { *(sph_u64 *)dst = val; }

static SPH_INLINE sph_u64
sph_dec64le(const void *src)                { return *(const sph_u64 *)src; }

static SPH_INLINE sph_u64
sph_dec64le_aligned(const void *src)        { return *(const sph_u64 *)src; }












#define AESx(x)   SPH_C32(x)
#define AES0      AES0_LE
#define AES1      AES1_LE
#define AES2      AES2_LE
#define AES3      AES3_LE

#define AES_ROUND_LE(X0, X1, X2, X3, K0, K1, K2, K3, Y0, Y1, Y2, Y3)   do { \
                (Y0) = AES0[(X0) & 0xFF] \
                        ^ AES1[((X1) >> 8) & 0xFF] \
                        ^ AES2[((X2) >> 16) & 0xFF] \
                        ^ AES3[((X3) >> 24) & 0xFF] ^ (K0); \
                (Y1) = AES0[(X1) & 0xFF] \
                        ^ AES1[((X2) >> 8) & 0xFF] \
                        ^ AES2[((X3) >> 16) & 0xFF] \
                        ^ AES3[((X0) >> 24) & 0xFF] ^ (K1); \
                (Y2) = AES0[(X2) & 0xFF] \
                        ^ AES1[((X3) >> 8) & 0xFF] \
                        ^ AES2[((X0) >> 16) & 0xFF] \
                        ^ AES3[((X1) >> 24) & 0xFF] ^ (K2); \
                (Y3) = AES0[(X3) & 0xFF] \
                        ^ AES1[((X0) >> 8) & 0xFF] \
                        ^ AES2[((X1) >> 16) & 0xFF] \
                        ^ AES3[((X2) >> 24) & 0xFF] ^ (K3); \
        } while (0)

#define AES_ROUND_NOKEY_LE(X0, X1, X2, X3, Y0, Y1, Y2, Y3) \
        AES_ROUND_LE(X0, X1, X2, X3, 0, 0, 0, 0, Y0, Y1, Y2, Y3)

static const sph_u32 AES0[256] = {
        AESx(0xA56363C6), AESx(0x847C7CF8), AESx(0x997777EE), AESx(0x8D7B7BF6),
        AESx(0x0DF2F2FF), AESx(0xBD6B6BD6), AESx(0xB16F6FDE), AESx(0x54C5C591),
        AESx(0x50303060), AESx(0x03010102), AESx(0xA96767CE), AESx(0x7D2B2B56),
        AESx(0x19FEFEE7), AESx(0x62D7D7B5), AESx(0xE6ABAB4D), AESx(0x9A7676EC),
        AESx(0x45CACA8F), AESx(0x9D82821F), AESx(0x40C9C989), AESx(0x877D7DFA),
        AESx(0x15FAFAEF), AESx(0xEB5959B2), AESx(0xC947478E), AESx(0x0BF0F0FB),
        AESx(0xECADAD41), AESx(0x67D4D4B3), AESx(0xFDA2A25F), AESx(0xEAAFAF45),
        AESx(0xBF9C9C23), AESx(0xF7A4A453), AESx(0x967272E4), AESx(0x5BC0C09B),
        AESx(0xC2B7B775), AESx(0x1CFDFDE1), AESx(0xAE93933D), AESx(0x6A26264C),
        AESx(0x5A36366C), AESx(0x413F3F7E), AESx(0x02F7F7F5), AESx(0x4FCCCC83),
        AESx(0x5C343468), AESx(0xF4A5A551), AESx(0x34E5E5D1), AESx(0x08F1F1F9),
        AESx(0x937171E2), AESx(0x73D8D8AB), AESx(0x53313162), AESx(0x3F15152A),
        AESx(0x0C040408), AESx(0x52C7C795), AESx(0x65232346), AESx(0x5EC3C39D),
        AESx(0x28181830), AESx(0xA1969637), AESx(0x0F05050A), AESx(0xB59A9A2F),
        AESx(0x0907070E), AESx(0x36121224), AESx(0x9B80801B), AESx(0x3DE2E2DF),
        AESx(0x26EBEBCD), AESx(0x6927274E), AESx(0xCDB2B27F), AESx(0x9F7575EA),
        AESx(0x1B090912), AESx(0x9E83831D), AESx(0x742C2C58), AESx(0x2E1A1A34),
        AESx(0x2D1B1B36), AESx(0xB26E6EDC), AESx(0xEE5A5AB4), AESx(0xFBA0A05B),
        AESx(0xF65252A4), AESx(0x4D3B3B76), AESx(0x61D6D6B7), AESx(0xCEB3B37D),
        AESx(0x7B292952), AESx(0x3EE3E3DD), AESx(0x712F2F5E), AESx(0x97848413),
        AESx(0xF55353A6), AESx(0x68D1D1B9), AESx(0x00000000), AESx(0x2CEDEDC1),
        AESx(0x60202040), AESx(0x1FFCFCE3), AESx(0xC8B1B179), AESx(0xED5B5BB6),
        AESx(0xBE6A6AD4), AESx(0x46CBCB8D), AESx(0xD9BEBE67), AESx(0x4B393972),
        AESx(0xDE4A4A94), AESx(0xD44C4C98), AESx(0xE85858B0), AESx(0x4ACFCF85),
        AESx(0x6BD0D0BB), AESx(0x2AEFEFC5), AESx(0xE5AAAA4F), AESx(0x16FBFBED),
        AESx(0xC5434386), AESx(0xD74D4D9A), AESx(0x55333366), AESx(0x94858511),
        AESx(0xCF45458A), AESx(0x10F9F9E9), AESx(0x06020204), AESx(0x817F7FFE),
        AESx(0xF05050A0), AESx(0x443C3C78), AESx(0xBA9F9F25), AESx(0xE3A8A84B),
        AESx(0xF35151A2), AESx(0xFEA3A35D), AESx(0xC0404080), AESx(0x8A8F8F05),
        AESx(0xAD92923F), AESx(0xBC9D9D21), AESx(0x48383870), AESx(0x04F5F5F1),
        AESx(0xDFBCBC63), AESx(0xC1B6B677), AESx(0x75DADAAF), AESx(0x63212142),
        AESx(0x30101020), AESx(0x1AFFFFE5), AESx(0x0EF3F3FD), AESx(0x6DD2D2BF),
        AESx(0x4CCDCD81), AESx(0x140C0C18), AESx(0x35131326), AESx(0x2FECECC3),
        AESx(0xE15F5FBE), AESx(0xA2979735), AESx(0xCC444488), AESx(0x3917172E),
        AESx(0x57C4C493), AESx(0xF2A7A755), AESx(0x827E7EFC), AESx(0x473D3D7A),
        AESx(0xAC6464C8), AESx(0xE75D5DBA), AESx(0x2B191932), AESx(0x957373E6),
        AESx(0xA06060C0), AESx(0x98818119), AESx(0xD14F4F9E), AESx(0x7FDCDCA3),
        AESx(0x66222244), AESx(0x7E2A2A54), AESx(0xAB90903B), AESx(0x8388880B),
        AESx(0xCA46468C), AESx(0x29EEEEC7), AESx(0xD3B8B86B), AESx(0x3C141428),
        AESx(0x79DEDEA7), AESx(0xE25E5EBC), AESx(0x1D0B0B16), AESx(0x76DBDBAD),
        AESx(0x3BE0E0DB), AESx(0x56323264), AESx(0x4E3A3A74), AESx(0x1E0A0A14),
        AESx(0xDB494992), AESx(0x0A06060C), AESx(0x6C242448), AESx(0xE45C5CB8),
        AESx(0x5DC2C29F), AESx(0x6ED3D3BD), AESx(0xEFACAC43), AESx(0xA66262C4),
        AESx(0xA8919139), AESx(0xA4959531), AESx(0x37E4E4D3), AESx(0x8B7979F2),
        AESx(0x32E7E7D5), AESx(0x43C8C88B), AESx(0x5937376E), AESx(0xB76D6DDA),
        AESx(0x8C8D8D01), AESx(0x64D5D5B1), AESx(0xD24E4E9C), AESx(0xE0A9A949),
        AESx(0xB46C6CD8), AESx(0xFA5656AC), AESx(0x07F4F4F3), AESx(0x25EAEACF),
        AESx(0xAF6565CA), AESx(0x8E7A7AF4), AESx(0xE9AEAE47), AESx(0x18080810),
        AESx(0xD5BABA6F), AESx(0x887878F0), AESx(0x6F25254A), AESx(0x722E2E5C),
        AESx(0x241C1C38), AESx(0xF1A6A657), AESx(0xC7B4B473), AESx(0x51C6C697),
        AESx(0x23E8E8CB), AESx(0x7CDDDDA1), AESx(0x9C7474E8), AESx(0x211F1F3E),
        AESx(0xDD4B4B96), AESx(0xDCBDBD61), AESx(0x868B8B0D), AESx(0x858A8A0F),
        AESx(0x907070E0), AESx(0x423E3E7C), AESx(0xC4B5B571), AESx(0xAA6666CC),
        AESx(0xD8484890), AESx(0x05030306), AESx(0x01F6F6F7), AESx(0x120E0E1C),
        AESx(0xA36161C2), AESx(0x5F35356A), AESx(0xF95757AE), AESx(0xD0B9B969),
        AESx(0x91868617), AESx(0x58C1C199), AESx(0x271D1D3A), AESx(0xB99E9E27),
        AESx(0x38E1E1D9), AESx(0x13F8F8EB), AESx(0xB398982B), AESx(0x33111122),
        AESx(0xBB6969D2), AESx(0x70D9D9A9), AESx(0x898E8E07), AESx(0xA7949433),
        AESx(0xB69B9B2D), AESx(0x221E1E3C), AESx(0x92878715), AESx(0x20E9E9C9),
        AESx(0x49CECE87), AESx(0xFF5555AA), AESx(0x78282850), AESx(0x7ADFDFA5),
        AESx(0x8F8C8C03), AESx(0xF8A1A159), AESx(0x80898909), AESx(0x170D0D1A),
        AESx(0xDABFBF65), AESx(0x31E6E6D7), AESx(0xC6424284), AESx(0xB86868D0),
        AESx(0xC3414182), AESx(0xB0999929), AESx(0x772D2D5A), AESx(0x110F0F1E),
        AESx(0xCBB0B07B), AESx(0xFC5454A8), AESx(0xD6BBBB6D), AESx(0x3A16162C)
};

static const sph_u32 AES1[256] = {
        AESx(0x6363C6A5), AESx(0x7C7CF884), AESx(0x7777EE99), AESx(0x7B7BF68D),
        AESx(0xF2F2FF0D), AESx(0x6B6BD6BD), AESx(0x6F6FDEB1), AESx(0xC5C59154),
        AESx(0x30306050), AESx(0x01010203), AESx(0x6767CEA9), AESx(0x2B2B567D),
        AESx(0xFEFEE719), AESx(0xD7D7B562), AESx(0xABAB4DE6), AESx(0x7676EC9A),
        AESx(0xCACA8F45), AESx(0x82821F9D), AESx(0xC9C98940), AESx(0x7D7DFA87),
        AESx(0xFAFAEF15), AESx(0x5959B2EB), AESx(0x47478EC9), AESx(0xF0F0FB0B),
        AESx(0xADAD41EC), AESx(0xD4D4B367), AESx(0xA2A25FFD), AESx(0xAFAF45EA),
        AESx(0x9C9C23BF), AESx(0xA4A453F7), AESx(0x7272E496), AESx(0xC0C09B5B),
        AESx(0xB7B775C2), AESx(0xFDFDE11C), AESx(0x93933DAE), AESx(0x26264C6A),
        AESx(0x36366C5A), AESx(0x3F3F7E41), AESx(0xF7F7F502), AESx(0xCCCC834F),
        AESx(0x3434685C), AESx(0xA5A551F4), AESx(0xE5E5D134), AESx(0xF1F1F908),
        AESx(0x7171E293), AESx(0xD8D8AB73), AESx(0x31316253), AESx(0x15152A3F),
        AESx(0x0404080C), AESx(0xC7C79552), AESx(0x23234665), AESx(0xC3C39D5E),
        AESx(0x18183028), AESx(0x969637A1), AESx(0x05050A0F), AESx(0x9A9A2FB5),
        AESx(0x07070E09), AESx(0x12122436), AESx(0x80801B9B), AESx(0xE2E2DF3D),
        AESx(0xEBEBCD26), AESx(0x27274E69), AESx(0xB2B27FCD), AESx(0x7575EA9F),
        AESx(0x0909121B), AESx(0x83831D9E), AESx(0x2C2C5874), AESx(0x1A1A342E),
        AESx(0x1B1B362D), AESx(0x6E6EDCB2), AESx(0x5A5AB4EE), AESx(0xA0A05BFB),
        AESx(0x5252A4F6), AESx(0x3B3B764D), AESx(0xD6D6B761), AESx(0xB3B37DCE),
        AESx(0x2929527B), AESx(0xE3E3DD3E), AESx(0x2F2F5E71), AESx(0x84841397),
        AESx(0x5353A6F5), AESx(0xD1D1B968), AESx(0x00000000), AESx(0xEDEDC12C),
        AESx(0x20204060), AESx(0xFCFCE31F), AESx(0xB1B179C8), AESx(0x5B5BB6ED),
        AESx(0x6A6AD4BE), AESx(0xCBCB8D46), AESx(0xBEBE67D9), AESx(0x3939724B),
        AESx(0x4A4A94DE), AESx(0x4C4C98D4), AESx(0x5858B0E8), AESx(0xCFCF854A),
        AESx(0xD0D0BB6B), AESx(0xEFEFC52A), AESx(0xAAAA4FE5), AESx(0xFBFBED16),
        AESx(0x434386C5), AESx(0x4D4D9AD7), AESx(0x33336655), AESx(0x85851194),
        AESx(0x45458ACF), AESx(0xF9F9E910), AESx(0x02020406), AESx(0x7F7FFE81),
        AESx(0x5050A0F0), AESx(0x3C3C7844), AESx(0x9F9F25BA), AESx(0xA8A84BE3),
        AESx(0x5151A2F3), AESx(0xA3A35DFE), AESx(0x404080C0), AESx(0x8F8F058A),
        AESx(0x92923FAD), AESx(0x9D9D21BC), AESx(0x38387048), AESx(0xF5F5F104),
        AESx(0xBCBC63DF), AESx(0xB6B677C1), AESx(0xDADAAF75), AESx(0x21214263),
        AESx(0x10102030), AESx(0xFFFFE51A), AESx(0xF3F3FD0E), AESx(0xD2D2BF6D),
        AESx(0xCDCD814C), AESx(0x0C0C1814), AESx(0x13132635), AESx(0xECECC32F),
        AESx(0x5F5FBEE1), AESx(0x979735A2), AESx(0x444488CC), AESx(0x17172E39),
        AESx(0xC4C49357), AESx(0xA7A755F2), AESx(0x7E7EFC82), AESx(0x3D3D7A47),
        AESx(0x6464C8AC), AESx(0x5D5DBAE7), AESx(0x1919322B), AESx(0x7373E695),
        AESx(0x6060C0A0), AESx(0x81811998), AESx(0x4F4F9ED1), AESx(0xDCDCA37F),
        AESx(0x22224466), AESx(0x2A2A547E), AESx(0x90903BAB), AESx(0x88880B83),
        AESx(0x46468CCA), AESx(0xEEEEC729), AESx(0xB8B86BD3), AESx(0x1414283C),
        AESx(0xDEDEA779), AESx(0x5E5EBCE2), AESx(0x0B0B161D), AESx(0xDBDBAD76),
        AESx(0xE0E0DB3B), AESx(0x32326456), AESx(0x3A3A744E), AESx(0x0A0A141E),
        AESx(0x494992DB), AESx(0x06060C0A), AESx(0x2424486C), AESx(0x5C5CB8E4),
        AESx(0xC2C29F5D), AESx(0xD3D3BD6E), AESx(0xACAC43EF), AESx(0x6262C4A6),
        AESx(0x919139A8), AESx(0x959531A4), AESx(0xE4E4D337), AESx(0x7979F28B),
        AESx(0xE7E7D532), AESx(0xC8C88B43), AESx(0x37376E59), AESx(0x6D6DDAB7),
        AESx(0x8D8D018C), AESx(0xD5D5B164), AESx(0x4E4E9CD2), AESx(0xA9A949E0),
        AESx(0x6C6CD8B4), AESx(0x5656ACFA), AESx(0xF4F4F307), AESx(0xEAEACF25),
        AESx(0x6565CAAF), AESx(0x7A7AF48E), AESx(0xAEAE47E9), AESx(0x08081018),
        AESx(0xBABA6FD5), AESx(0x7878F088), AESx(0x25254A6F), AESx(0x2E2E5C72),
        AESx(0x1C1C3824), AESx(0xA6A657F1), AESx(0xB4B473C7), AESx(0xC6C69751),
        AESx(0xE8E8CB23), AESx(0xDDDDA17C), AESx(0x7474E89C), AESx(0x1F1F3E21),
        AESx(0x4B4B96DD), AESx(0xBDBD61DC), AESx(0x8B8B0D86), AESx(0x8A8A0F85),
        AESx(0x7070E090), AESx(0x3E3E7C42), AESx(0xB5B571C4), AESx(0x6666CCAA),
        AESx(0x484890D8), AESx(0x03030605), AESx(0xF6F6F701), AESx(0x0E0E1C12),
        AESx(0x6161C2A3), AESx(0x35356A5F), AESx(0x5757AEF9), AESx(0xB9B969D0),
        AESx(0x86861791), AESx(0xC1C19958), AESx(0x1D1D3A27), AESx(0x9E9E27B9),
        AESx(0xE1E1D938), AESx(0xF8F8EB13), AESx(0x98982BB3), AESx(0x11112233),
        AESx(0x6969D2BB), AESx(0xD9D9A970), AESx(0x8E8E0789), AESx(0x949433A7),
        AESx(0x9B9B2DB6), AESx(0x1E1E3C22), AESx(0x87871592), AESx(0xE9E9C920),
        AESx(0xCECE8749), AESx(0x5555AAFF), AESx(0x28285078), AESx(0xDFDFA57A),
        AESx(0x8C8C038F), AESx(0xA1A159F8), AESx(0x89890980), AESx(0x0D0D1A17),
        AESx(0xBFBF65DA), AESx(0xE6E6D731), AESx(0x424284C6), AESx(0x6868D0B8),
        AESx(0x414182C3), AESx(0x999929B0), AESx(0x2D2D5A77), AESx(0x0F0F1E11),
        AESx(0xB0B07BCB), AESx(0x5454A8FC), AESx(0xBBBB6DD6), AESx(0x16162C3A)
};

static const sph_u32 AES2[256] = {
        AESx(0x63C6A563), AESx(0x7CF8847C), AESx(0x77EE9977), AESx(0x7BF68D7B),
        AESx(0xF2FF0DF2), AESx(0x6BD6BD6B), AESx(0x6FDEB16F), AESx(0xC59154C5),
        AESx(0x30605030), AESx(0x01020301), AESx(0x67CEA967), AESx(0x2B567D2B),
        AESx(0xFEE719FE), AESx(0xD7B562D7), AESx(0xAB4DE6AB), AESx(0x76EC9A76),
        AESx(0xCA8F45CA), AESx(0x821F9D82), AESx(0xC98940C9), AESx(0x7DFA877D),
        AESx(0xFAEF15FA), AESx(0x59B2EB59), AESx(0x478EC947), AESx(0xF0FB0BF0),
        AESx(0xAD41ECAD), AESx(0xD4B367D4), AESx(0xA25FFDA2), AESx(0xAF45EAAF),
        AESx(0x9C23BF9C), AESx(0xA453F7A4), AESx(0x72E49672), AESx(0xC09B5BC0),
        AESx(0xB775C2B7), AESx(0xFDE11CFD), AESx(0x933DAE93), AESx(0x264C6A26),
        AESx(0x366C5A36), AESx(0x3F7E413F), AESx(0xF7F502F7), AESx(0xCC834FCC),
        AESx(0x34685C34), AESx(0xA551F4A5), AESx(0xE5D134E5), AESx(0xF1F908F1),
        AESx(0x71E29371), AESx(0xD8AB73D8), AESx(0x31625331), AESx(0x152A3F15),
        AESx(0x04080C04), AESx(0xC79552C7), AESx(0x23466523), AESx(0xC39D5EC3),
        AESx(0x18302818), AESx(0x9637A196), AESx(0x050A0F05), AESx(0x9A2FB59A),
        AESx(0x070E0907), AESx(0x12243612), AESx(0x801B9B80), AESx(0xE2DF3DE2),
        AESx(0xEBCD26EB), AESx(0x274E6927), AESx(0xB27FCDB2), AESx(0x75EA9F75),
        AESx(0x09121B09), AESx(0x831D9E83), AESx(0x2C58742C), AESx(0x1A342E1A),
        AESx(0x1B362D1B), AESx(0x6EDCB26E), AESx(0x5AB4EE5A), AESx(0xA05BFBA0),
        AESx(0x52A4F652), AESx(0x3B764D3B), AESx(0xD6B761D6), AESx(0xB37DCEB3),
        AESx(0x29527B29), AESx(0xE3DD3EE3), AESx(0x2F5E712F), AESx(0x84139784),
        AESx(0x53A6F553), AESx(0xD1B968D1), AESx(0x00000000), AESx(0xEDC12CED),
        AESx(0x20406020), AESx(0xFCE31FFC), AESx(0xB179C8B1), AESx(0x5BB6ED5B),
        AESx(0x6AD4BE6A), AESx(0xCB8D46CB), AESx(0xBE67D9BE), AESx(0x39724B39),
        AESx(0x4A94DE4A), AESx(0x4C98D44C), AESx(0x58B0E858), AESx(0xCF854ACF),
        AESx(0xD0BB6BD0), AESx(0xEFC52AEF), AESx(0xAA4FE5AA), AESx(0xFBED16FB),
        AESx(0x4386C543), AESx(0x4D9AD74D), AESx(0x33665533), AESx(0x85119485),
        AESx(0x458ACF45), AESx(0xF9E910F9), AESx(0x02040602), AESx(0x7FFE817F),
        AESx(0x50A0F050), AESx(0x3C78443C), AESx(0x9F25BA9F), AESx(0xA84BE3A8),
        AESx(0x51A2F351), AESx(0xA35DFEA3), AESx(0x4080C040), AESx(0x8F058A8F),
        AESx(0x923FAD92), AESx(0x9D21BC9D), AESx(0x38704838), AESx(0xF5F104F5),
        AESx(0xBC63DFBC), AESx(0xB677C1B6), AESx(0xDAAF75DA), AESx(0x21426321),
        AESx(0x10203010), AESx(0xFFE51AFF), AESx(0xF3FD0EF3), AESx(0xD2BF6DD2),
        AESx(0xCD814CCD), AESx(0x0C18140C), AESx(0x13263513), AESx(0xECC32FEC),
        AESx(0x5FBEE15F), AESx(0x9735A297), AESx(0x4488CC44), AESx(0x172E3917),
        AESx(0xC49357C4), AESx(0xA755F2A7), AESx(0x7EFC827E), AESx(0x3D7A473D),
        AESx(0x64C8AC64), AESx(0x5DBAE75D), AESx(0x19322B19), AESx(0x73E69573),
        AESx(0x60C0A060), AESx(0x81199881), AESx(0x4F9ED14F), AESx(0xDCA37FDC),
        AESx(0x22446622), AESx(0x2A547E2A), AESx(0x903BAB90), AESx(0x880B8388),
        AESx(0x468CCA46), AESx(0xEEC729EE), AESx(0xB86BD3B8), AESx(0x14283C14),
        AESx(0xDEA779DE), AESx(0x5EBCE25E), AESx(0x0B161D0B), AESx(0xDBAD76DB),
        AESx(0xE0DB3BE0), AESx(0x32645632), AESx(0x3A744E3A), AESx(0x0A141E0A),
        AESx(0x4992DB49), AESx(0x060C0A06), AESx(0x24486C24), AESx(0x5CB8E45C),
        AESx(0xC29F5DC2), AESx(0xD3BD6ED3), AESx(0xAC43EFAC), AESx(0x62C4A662),
        AESx(0x9139A891), AESx(0x9531A495), AESx(0xE4D337E4), AESx(0x79F28B79),
        AESx(0xE7D532E7), AESx(0xC88B43C8), AESx(0x376E5937), AESx(0x6DDAB76D),
        AESx(0x8D018C8D), AESx(0xD5B164D5), AESx(0x4E9CD24E), AESx(0xA949E0A9),
        AESx(0x6CD8B46C), AESx(0x56ACFA56), AESx(0xF4F307F4), AESx(0xEACF25EA),
        AESx(0x65CAAF65), AESx(0x7AF48E7A), AESx(0xAE47E9AE), AESx(0x08101808),
        AESx(0xBA6FD5BA), AESx(0x78F08878), AESx(0x254A6F25), AESx(0x2E5C722E),
        AESx(0x1C38241C), AESx(0xA657F1A6), AESx(0xB473C7B4), AESx(0xC69751C6),
        AESx(0xE8CB23E8), AESx(0xDDA17CDD), AESx(0x74E89C74), AESx(0x1F3E211F),
        AESx(0x4B96DD4B), AESx(0xBD61DCBD), AESx(0x8B0D868B), AESx(0x8A0F858A),
        AESx(0x70E09070), AESx(0x3E7C423E), AESx(0xB571C4B5), AESx(0x66CCAA66),
        AESx(0x4890D848), AESx(0x03060503), AESx(0xF6F701F6), AESx(0x0E1C120E),
        AESx(0x61C2A361), AESx(0x356A5F35), AESx(0x57AEF957), AESx(0xB969D0B9),
        AESx(0x86179186), AESx(0xC19958C1), AESx(0x1D3A271D), AESx(0x9E27B99E),
        AESx(0xE1D938E1), AESx(0xF8EB13F8), AESx(0x982BB398), AESx(0x11223311),
        AESx(0x69D2BB69), AESx(0xD9A970D9), AESx(0x8E07898E), AESx(0x9433A794),
        AESx(0x9B2DB69B), AESx(0x1E3C221E), AESx(0x87159287), AESx(0xE9C920E9),
        AESx(0xCE8749CE), AESx(0x55AAFF55), AESx(0x28507828), AESx(0xDFA57ADF),
        AESx(0x8C038F8C), AESx(0xA159F8A1), AESx(0x89098089), AESx(0x0D1A170D),
        AESx(0xBF65DABF), AESx(0xE6D731E6), AESx(0x4284C642), AESx(0x68D0B868),
        AESx(0x4182C341), AESx(0x9929B099), AESx(0x2D5A772D), AESx(0x0F1E110F),
        AESx(0xB07BCBB0), AESx(0x54A8FC54), AESx(0xBB6DD6BB), AESx(0x162C3A16)
};

static const sph_u32 AES3[256] = {
        AESx(0xC6A56363), AESx(0xF8847C7C), AESx(0xEE997777), AESx(0xF68D7B7B),
        AESx(0xFF0DF2F2), AESx(0xD6BD6B6B), AESx(0xDEB16F6F), AESx(0x9154C5C5),
        AESx(0x60503030), AESx(0x02030101), AESx(0xCEA96767), AESx(0x567D2B2B),
        AESx(0xE719FEFE), AESx(0xB562D7D7), AESx(0x4DE6ABAB), AESx(0xEC9A7676),
        AESx(0x8F45CACA), AESx(0x1F9D8282), AESx(0x8940C9C9), AESx(0xFA877D7D),
        AESx(0xEF15FAFA), AESx(0xB2EB5959), AESx(0x8EC94747), AESx(0xFB0BF0F0),
        AESx(0x41ECADAD), AESx(0xB367D4D4), AESx(0x5FFDA2A2), AESx(0x45EAAFAF),
        AESx(0x23BF9C9C), AESx(0x53F7A4A4), AESx(0xE4967272), AESx(0x9B5BC0C0),
        AESx(0x75C2B7B7), AESx(0xE11CFDFD), AESx(0x3DAE9393), AESx(0x4C6A2626),
        AESx(0x6C5A3636), AESx(0x7E413F3F), AESx(0xF502F7F7), AESx(0x834FCCCC),
        AESx(0x685C3434), AESx(0x51F4A5A5), AESx(0xD134E5E5), AESx(0xF908F1F1),
        AESx(0xE2937171), AESx(0xAB73D8D8), AESx(0x62533131), AESx(0x2A3F1515),
        AESx(0x080C0404), AESx(0x9552C7C7), AESx(0x46652323), AESx(0x9D5EC3C3),
        AESx(0x30281818), AESx(0x37A19696), AESx(0x0A0F0505), AESx(0x2FB59A9A),
        AESx(0x0E090707), AESx(0x24361212), AESx(0x1B9B8080), AESx(0xDF3DE2E2),
        AESx(0xCD26EBEB), AESx(0x4E692727), AESx(0x7FCDB2B2), AESx(0xEA9F7575),
        AESx(0x121B0909), AESx(0x1D9E8383), AESx(0x58742C2C), AESx(0x342E1A1A),
        AESx(0x362D1B1B), AESx(0xDCB26E6E), AESx(0xB4EE5A5A), AESx(0x5BFBA0A0),
        AESx(0xA4F65252), AESx(0x764D3B3B), AESx(0xB761D6D6), AESx(0x7DCEB3B3),
        AESx(0x527B2929), AESx(0xDD3EE3E3), AESx(0x5E712F2F), AESx(0x13978484),
        AESx(0xA6F55353), AESx(0xB968D1D1), AESx(0x00000000), AESx(0xC12CEDED),
        AESx(0x40602020), AESx(0xE31FFCFC), AESx(0x79C8B1B1), AESx(0xB6ED5B5B),
        AESx(0xD4BE6A6A), AESx(0x8D46CBCB), AESx(0x67D9BEBE), AESx(0x724B3939),
        AESx(0x94DE4A4A), AESx(0x98D44C4C), AESx(0xB0E85858), AESx(0x854ACFCF),
        AESx(0xBB6BD0D0), AESx(0xC52AEFEF), AESx(0x4FE5AAAA), AESx(0xED16FBFB),
        AESx(0x86C54343), AESx(0x9AD74D4D), AESx(0x66553333), AESx(0x11948585),
        AESx(0x8ACF4545), AESx(0xE910F9F9), AESx(0x04060202), AESx(0xFE817F7F),
        AESx(0xA0F05050), AESx(0x78443C3C), AESx(0x25BA9F9F), AESx(0x4BE3A8A8),
        AESx(0xA2F35151), AESx(0x5DFEA3A3), AESx(0x80C04040), AESx(0x058A8F8F),
        AESx(0x3FAD9292), AESx(0x21BC9D9D), AESx(0x70483838), AESx(0xF104F5F5),
        AESx(0x63DFBCBC), AESx(0x77C1B6B6), AESx(0xAF75DADA), AESx(0x42632121),
        AESx(0x20301010), AESx(0xE51AFFFF), AESx(0xFD0EF3F3), AESx(0xBF6DD2D2),
        AESx(0x814CCDCD), AESx(0x18140C0C), AESx(0x26351313), AESx(0xC32FECEC),
        AESx(0xBEE15F5F), AESx(0x35A29797), AESx(0x88CC4444), AESx(0x2E391717),
        AESx(0x9357C4C4), AESx(0x55F2A7A7), AESx(0xFC827E7E), AESx(0x7A473D3D),
        AESx(0xC8AC6464), AESx(0xBAE75D5D), AESx(0x322B1919), AESx(0xE6957373),
        AESx(0xC0A06060), AESx(0x19988181), AESx(0x9ED14F4F), AESx(0xA37FDCDC),
        AESx(0x44662222), AESx(0x547E2A2A), AESx(0x3BAB9090), AESx(0x0B838888),
        AESx(0x8CCA4646), AESx(0xC729EEEE), AESx(0x6BD3B8B8), AESx(0x283C1414),
        AESx(0xA779DEDE), AESx(0xBCE25E5E), AESx(0x161D0B0B), AESx(0xAD76DBDB),
        AESx(0xDB3BE0E0), AESx(0x64563232), AESx(0x744E3A3A), AESx(0x141E0A0A),
        AESx(0x92DB4949), AESx(0x0C0A0606), AESx(0x486C2424), AESx(0xB8E45C5C),
        AESx(0x9F5DC2C2), AESx(0xBD6ED3D3), AESx(0x43EFACAC), AESx(0xC4A66262),
        AESx(0x39A89191), AESx(0x31A49595), AESx(0xD337E4E4), AESx(0xF28B7979),
        AESx(0xD532E7E7), AESx(0x8B43C8C8), AESx(0x6E593737), AESx(0xDAB76D6D),
        AESx(0x018C8D8D), AESx(0xB164D5D5), AESx(0x9CD24E4E), AESx(0x49E0A9A9),
        AESx(0xD8B46C6C), AESx(0xACFA5656), AESx(0xF307F4F4), AESx(0xCF25EAEA),
        AESx(0xCAAF6565), AESx(0xF48E7A7A), AESx(0x47E9AEAE), AESx(0x10180808),
        AESx(0x6FD5BABA), AESx(0xF0887878), AESx(0x4A6F2525), AESx(0x5C722E2E),
        AESx(0x38241C1C), AESx(0x57F1A6A6), AESx(0x73C7B4B4), AESx(0x9751C6C6),
        AESx(0xCB23E8E8), AESx(0xA17CDDDD), AESx(0xE89C7474), AESx(0x3E211F1F),
        AESx(0x96DD4B4B), AESx(0x61DCBDBD), AESx(0x0D868B8B), AESx(0x0F858A8A),
        AESx(0xE0907070), AESx(0x7C423E3E), AESx(0x71C4B5B5), AESx(0xCCAA6666),
        AESx(0x90D84848), AESx(0x06050303), AESx(0xF701F6F6), AESx(0x1C120E0E),
        AESx(0xC2A36161), AESx(0x6A5F3535), AESx(0xAEF95757), AESx(0x69D0B9B9),
        AESx(0x17918686), AESx(0x9958C1C1), AESx(0x3A271D1D), AESx(0x27B99E9E),
        AESx(0xD938E1E1), AESx(0xEB13F8F8), AESx(0x2BB39898), AESx(0x22331111),
        AESx(0xD2BB6969), AESx(0xA970D9D9), AESx(0x07898E8E), AESx(0x33A79494),
        AESx(0x2DB69B9B), AESx(0x3C221E1E), AESx(0x15928787), AESx(0xC920E9E9),
        AESx(0x8749CECE), AESx(0xAAFF5555), AESx(0x50782828), AESx(0xA57ADFDF),
        AESx(0x038F8C8C), AESx(0x59F8A1A1), AESx(0x09808989), AESx(0x1A170D0D),
        AESx(0x65DABFBF), AESx(0xD731E6E6), AESx(0x84C64242), AESx(0xD0B86868),
        AESx(0x82C34141), AESx(0x29B09999), AESx(0x5A772D2D), AESx(0x1E110F0F),
        AESx(0x7BCBB0B0), AESx(0xA8FC5454), AESx(0x6DD6BBBB), AESx(0x2C3A1616)
};
























#define SPH_SIZE_bmw224   224
#define SPH_SIZE_bmw256   256
#define SPH_SIZE_bmw384   384
#define SPH_SIZE_bmw512   512

typedef struct {
        unsigned char buf[64];    /* first field, for alignment */
        size_t ptr;
        sph_u32 H[16];
        sph_u64 bit_count;
} sph_bmw_small_context;


typedef struct {
        unsigned char buf[128];    /* first field, for alignment */
        size_t ptr;
        sph_u64 H[16];
        sph_u64 bit_count;
} sph_bmw_big_context;

void
sph_bmw224_init(sph_bmw_small_context *cc);
void
sph_bmw224(sph_bmw_small_context *cc, const unsigned char *data, size_t len);
void
sph_bmw224_close(sph_bmw_small_context *cc, unsigned char *dst);
void
sph_bmw224_addbits_and_close(sph_bmw_small_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_bmw256_init(sph_bmw_small_context *cc);
void
sph_bmw256(sph_bmw_small_context *cc, const unsigned char *data, size_t len);
void
sph_bmw256_close(sph_bmw_small_context *cc, unsigned char *dst);
void
sph_bmw256_addbits_and_close(sph_bmw_small_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_bmw384_init(sph_bmw_big_context *cc);
void
sph_bmw384(sph_bmw_big_context *cc, const unsigned char *data, size_t len);
void
sph_bmw384_close(sph_bmw_big_context *cc, unsigned char *dst);
void
sph_bmw384_addbits_and_close(sph_bmw_big_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_bmw512_init(sph_bmw_big_context *cc);
void
sph_bmw512(sph_bmw_big_context *cc, const unsigned char *data, size_t len);
void
sph_bmw512_close(sph_bmw_big_context *cc, unsigned char *dst);
void
sph_bmw512_addbits_and_close(sph_bmw_big_context *cc, unsigned ub, unsigned n, unsigned char *dst);






static const sph_u32 bmw_IV224[] = {
        SPH_C32(0x00010203), SPH_C32(0x04050607),
        SPH_C32(0x08090A0B), SPH_C32(0x0C0D0E0F),
        SPH_C32(0x10111213), SPH_C32(0x14151617),
        SPH_C32(0x18191A1B), SPH_C32(0x1C1D1E1F),
        SPH_C32(0x20212223), SPH_C32(0x24252627),
        SPH_C32(0x28292A2B), SPH_C32(0x2C2D2E2F),
        SPH_C32(0x30313233), SPH_C32(0x34353637),
        SPH_C32(0x38393A3B), SPH_C32(0x3C3D3E3F)
};

static const sph_u32 bmw_IV256[] = {
        SPH_C32(0x40414243), SPH_C32(0x44454647),
        SPH_C32(0x48494A4B), SPH_C32(0x4C4D4E4F),
        SPH_C32(0x50515253), SPH_C32(0x54555657),
        SPH_C32(0x58595A5B), SPH_C32(0x5C5D5E5F),
        SPH_C32(0x60616263), SPH_C32(0x64656667),
        SPH_C32(0x68696A6B), SPH_C32(0x6C6D6E6F),
        SPH_C32(0x70717273), SPH_C32(0x74757677),
        SPH_C32(0x78797A7B), SPH_C32(0x7C7D7E7F)
};


static const sph_u64 bmw_IV384[] = {
        SPH_C64(0x0001020304050607), SPH_C64(0x08090A0B0C0D0E0F),
        SPH_C64(0x1011121314151617), SPH_C64(0x18191A1B1C1D1E1F),
        SPH_C64(0x2021222324252627), SPH_C64(0x28292A2B2C2D2E2F),
        SPH_C64(0x3031323334353637), SPH_C64(0x38393A3B3C3D3E3F),
        SPH_C64(0x4041424344454647), SPH_C64(0x48494A4B4C4D4E4F),
        SPH_C64(0x5051525354555657), SPH_C64(0x58595A5B5C5D5E5F),
        SPH_C64(0x6061626364656667), SPH_C64(0x68696A6B6C6D6E6F),
        SPH_C64(0x7071727374757677), SPH_C64(0x78797A7B7C7D7E7F)
};

static const sph_u64 bmw_IV512[] = {
        SPH_C64(0x8081828384858687), SPH_C64(0x88898A8B8C8D8E8F),
        SPH_C64(0x9091929394959697), SPH_C64(0x98999A9B9C9D9E9F),
        SPH_C64(0xA0A1A2A3A4A5A6A7), SPH_C64(0xA8A9AAABACADAEAF),
        SPH_C64(0xB0B1B2B3B4B5B6B7), SPH_C64(0xB8B9BABBBCBDBEBF),
        SPH_C64(0xC0C1C2C3C4C5C6C7), SPH_C64(0xC8C9CACBCCCDCECF),
        SPH_C64(0xD0D1D2D3D4D5D6D7), SPH_C64(0xD8D9DADBDCDDDEDF),
        SPH_C64(0xE0E1E2E3E4E5E6E7), SPH_C64(0xE8E9EAEBECEDEEEF),
        SPH_C64(0xF0F1F2F3F4F5F6F7), SPH_C64(0xF8F9FAFBFCFDFEFF)
};


#define XCAT(x, y)    XCAT_(x, y)
#define XCAT_(x, y)   x ## y

#define LPAR   (

#define I16_16    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
#define I16_17    1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16
#define I16_18    2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17
#define I16_19    3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18
#define I16_20    4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
#define I16_21    5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
#define I16_22    6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
#define I16_23    7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
#define I16_24    8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
#define I16_25    9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
#define I16_26   10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
#define I16_27   11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26
#define I16_28   12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27
#define I16_29   13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
#define I16_30   14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
#define I16_31   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30

#define M16_16    0,  1,  3,  4,  7, 10, 11
#define M16_17    1,  2,  4,  5,  8, 11, 12
#define M16_18    2,  3,  5,  6,  9, 12, 13
#define M16_19    3,  4,  6,  7, 10, 13, 14
#define M16_20    4,  5,  7,  8, 11, 14, 15
#define M16_21    5,  6,  8,  9, 12, 15, 16
#define M16_22    6,  7,  9, 10, 13,  0,  1
#define M16_23    7,  8, 10, 11, 14,  1,  2
#define M16_24    8,  9, 11, 12, 15,  2,  3
#define M16_25    9, 10, 12, 13,  0,  3,  4
#define M16_26   10, 11, 13, 14,  1,  4,  5
#define M16_27   11, 12, 14, 15,  2,  5,  6
#define M16_28   12, 13, 15, 16,  3,  6,  7
#define M16_29   13, 14,  0,  1,  4,  7,  8
#define M16_30   14, 15,  1,  2,  5,  8,  9
#define M16_31   15, 16,  2,  3,  6,  9, 10

#define ss0(x)    (((x) >> 1) ^ SPH_T32((x) << 3) \
                  ^ SPH_ROTL32(x,  4) ^ SPH_ROTL32(x, 19))
#define ss1(x)    (((x) >> 1) ^ SPH_T32((x) << 2) \
                  ^ SPH_ROTL32(x,  8) ^ SPH_ROTL32(x, 23))
#define ss2(x)    (((x) >> 2) ^ SPH_T32((x) << 1) \
                  ^ SPH_ROTL32(x, 12) ^ SPH_ROTL32(x, 25))
#define ss3(x)    (((x) >> 2) ^ SPH_T32((x) << 2) \
                  ^ SPH_ROTL32(x, 15) ^ SPH_ROTL32(x, 29))
#define ss4(x)    (((x) >> 1) ^ (x))
#define ss5(x)    (((x) >> 2) ^ (x))
#define rs1(x)    SPH_ROTL32(x,  3)
#define rs2(x)    SPH_ROTL32(x,  7)
#define rs3(x)    SPH_ROTL32(x, 13)
#define rs4(x)    SPH_ROTL32(x, 16)
#define rs5(x)    SPH_ROTL32(x, 19)
#define rs6(x)    SPH_ROTL32(x, 23)
#define rs7(x)    SPH_ROTL32(x, 27)

#define Ks(j)   SPH_T32((sph_u32)(j) * SPH_C32(0x05555555))

#define add_elt_s(mf, hf, j0m, j1m, j3m, j4m, j7m, j10m, j11m, j16) \
        (SPH_T32(SPH_ROTL32(mf(j0m), j1m) + SPH_ROTL32(mf(j3m), j4m) \
                - SPH_ROTL32(mf(j10m), j11m) + Ks(j16)) ^ hf(j7m))

#define expand1s_inner(qf, mf, hf, i16, \
                i0, i1, i2, i3, i4, i5, i6, i7, i8, \
                i9, i10, i11, i12, i13, i14, i15, \
                i0m, i1m, i3m, i4m, i7m, i10m, i11m) \
        SPH_T32(ss1(qf(i0)) + ss2(qf(i1)) + ss3(qf(i2)) + ss0(qf(i3)) \
                + ss1(qf(i4)) + ss2(qf(i5)) + ss3(qf(i6)) + ss0(qf(i7)) \
                + ss1(qf(i8)) + ss2(qf(i9)) + ss3(qf(i10)) + ss0(qf(i11)) \
                + ss1(qf(i12)) + ss2(qf(i13)) + ss3(qf(i14)) + ss0(qf(i15)) \
                + add_elt_s(mf, hf, i0m, i1m, i3m, i4m, i7m, i10m, i11m, i16))

#define expand1s(qf, mf, hf, i16) \
        expand1s_(qf, mf, hf, i16, I16_ ## i16, M16_ ## i16)
#define expand1s_(qf, mf, hf, i16, ix, iy) \
        expand1s_inner LPAR qf, mf, hf, i16, ix, iy)

#define expand2s_inner(qf, mf, hf, i16, \
                i0, i1, i2, i3, i4, i5, i6, i7, i8, \
                i9, i10, i11, i12, i13, i14, i15, \
                i0m, i1m, i3m, i4m, i7m, i10m, i11m) \
        SPH_T32(qf(i0) + rs1(qf(i1)) + qf(i2) + rs2(qf(i3)) \
                + qf(i4) + rs3(qf(i5)) + qf(i6) + rs4(qf(i7)) \
                + qf(i8) + rs5(qf(i9)) + qf(i10) + rs6(qf(i11)) \
                + qf(i12) + rs7(qf(i13)) + ss4(qf(i14)) + ss5(qf(i15)) \
                + add_elt_s(mf, hf, i0m, i1m, i3m, i4m, i7m, i10m, i11m, i16))

#define expand2s(qf, mf, hf, i16) \
        expand2s_(qf, mf, hf, i16, I16_ ## i16, M16_ ## i16)
#define expand2s_(qf, mf, hf, i16, ix, iy) \
        expand2s_inner LPAR qf, mf, hf, i16, ix, iy)

#define sb0(x)    (((x) >> 1) ^ SPH_T64((x) << 3) \
                  ^ SPH_ROTL64(x,  4) ^ SPH_ROTL64(x, 37))
#define sb1(x)    (((x) >> 1) ^ SPH_T64((x) << 2) \
                  ^ SPH_ROTL64(x, 13) ^ SPH_ROTL64(x, 43))
#define sb2(x)    (((x) >> 2) ^ SPH_T64((x) << 1) \
                  ^ SPH_ROTL64(x, 19) ^ SPH_ROTL64(x, 53))
#define sb3(x)    (((x) >> 2) ^ SPH_T64((x) << 2) \
                  ^ SPH_ROTL64(x, 28) ^ SPH_ROTL64(x, 59))
#define sb4(x)    (((x) >> 1) ^ (x))
#define sb5(x)    (((x) >> 2) ^ (x))
#define rb1(x)    SPH_ROTL64(x,  5)
#define rb2(x)    SPH_ROTL64(x, 11)
#define rb3(x)    SPH_ROTL64(x, 27)
#define rb4(x)    SPH_ROTL64(x, 32)
#define rb5(x)    SPH_ROTL64(x, 37)
#define rb6(x)    SPH_ROTL64(x, 43)
#define rb7(x)    SPH_ROTL64(x, 53)

#define Kb(j)   SPH_T64((sph_u64)(j) * SPH_C64(0x0555555555555555))


#define add_elt_b(mf, hf, j0m, j1m, j3m, j4m, j7m, j10m, j11m, j16) \
        (SPH_T64(SPH_ROTL64(mf(j0m), j1m) + SPH_ROTL64(mf(j3m), j4m) \
                - SPH_ROTL64(mf(j10m), j11m) + Kb(j16)) ^ hf(j7m))

#define expand1b_inner(qf, mf, hf, i16, \
                i0, i1, i2, i3, i4, i5, i6, i7, i8, \
                i9, i10, i11, i12, i13, i14, i15, \
                i0m, i1m, i3m, i4m, i7m, i10m, i11m) \
        SPH_T64(sb1(qf(i0)) + sb2(qf(i1)) + sb3(qf(i2)) + sb0(qf(i3)) \
                + sb1(qf(i4)) + sb2(qf(i5)) + sb3(qf(i6)) + sb0(qf(i7)) \
                + sb1(qf(i8)) + sb2(qf(i9)) + sb3(qf(i10)) + sb0(qf(i11)) \
                + sb1(qf(i12)) + sb2(qf(i13)) + sb3(qf(i14)) + sb0(qf(i15)) \
                + add_elt_b(mf, hf, i0m, i1m, i3m, i4m, i7m, i10m, i11m, i16))

#define expand1b(qf, mf, hf, i16) \
        expand1b_(qf, mf, hf, i16, I16_ ## i16, M16_ ## i16)
#define expand1b_(qf, mf, hf, i16, ix, iy) \
        expand1b_inner LPAR qf, mf, hf, i16, ix, iy)

#define expand2b_inner(qf, mf, hf, i16, \
                i0, i1, i2, i3, i4, i5, i6, i7, i8, \
                i9, i10, i11, i12, i13, i14, i15, \
                i0m, i1m, i3m, i4m, i7m, i10m, i11m) \
        SPH_T64(qf(i0) + rb1(qf(i1)) + qf(i2) + rb2(qf(i3)) \
                + qf(i4) + rb3(qf(i5)) + qf(i6) + rb4(qf(i7)) \
                + qf(i8) + rb5(qf(i9)) + qf(i10) + rb6(qf(i11)) \
                + qf(i12) + rb7(qf(i13)) + sb4(qf(i14)) + sb5(qf(i15)) \
                + add_elt_b(mf, hf, i0m, i1m, i3m, i4m, i7m, i10m, i11m, i16))

#define expand2b(qf, mf, hf, i16) \
        expand2b_(qf, mf, hf, i16, I16_ ## i16, M16_ ## i16)
#define expand2b_(qf, mf, hf, i16, ix, iy) \
        expand2b_inner LPAR qf, mf, hf, i16, ix, iy)



#define MAKE_W(tt, i0, op01, i1, op12, i2, op23, i3, op34, i4) \
        tt((M(i0) ^ H(i0)) op01 (M(i1) ^ H(i1)) op12 (M(i2) ^ H(i2)) \
        op23 (M(i3) ^ H(i3)) op34 (M(i4) ^ H(i4)))

#define Ws0    MAKE_W(SPH_T32,  5, -,  7, +, 10, +, 13, +, 14)
#define Ws1    MAKE_W(SPH_T32,  6, -,  8, +, 11, +, 14, -, 15)
#define Ws2    MAKE_W(SPH_T32,  0, +,  7, +,  9, -, 12, +, 15)
#define Ws3    MAKE_W(SPH_T32,  0, -,  1, +,  8, -, 10, +, 13)
#define Ws4    MAKE_W(SPH_T32,  1, +,  2, +,  9, -, 11, -, 14)
#define Ws5    MAKE_W(SPH_T32,  3, -,  2, +, 10, -, 12, +, 15)
#define Ws6    MAKE_W(SPH_T32,  4, -,  0, -,  3, -, 11, +, 13)
#define Ws7    MAKE_W(SPH_T32,  1, -,  4, -,  5, -, 12, -, 14)
#define Ws8    MAKE_W(SPH_T32,  2, -,  5, -,  6, +, 13, -, 15)
#define Ws9    MAKE_W(SPH_T32,  0, -,  3, +,  6, -,  7, +, 14)
#define Ws10   MAKE_W(SPH_T32,  8, -,  1, -,  4, -,  7, +, 15)
#define Ws11   MAKE_W(SPH_T32,  8, -,  0, -,  2, -,  5, +,  9)
#define Ws12   MAKE_W(SPH_T32,  1, +,  3, -,  6, -,  9, +, 10)
#define Ws13   MAKE_W(SPH_T32,  2, +,  4, +,  7, +, 10, +, 11)
#define Ws14   MAKE_W(SPH_T32,  3, -,  5, +,  8, -, 11, -, 12)
#define Ws15   MAKE_W(SPH_T32, 12, -,  4, -,  6, -,  9, +, 13)

#define MAKE_Qas   do { \
                qt[ 0] = SPH_T32(ss0(Ws0 ) + H( 1)); \
                qt[ 1] = SPH_T32(ss1(Ws1 ) + H( 2)); \
                qt[ 2] = SPH_T32(ss2(Ws2 ) + H( 3)); \
                qt[ 3] = SPH_T32(ss3(Ws3 ) + H( 4)); \
                qt[ 4] = SPH_T32(ss4(Ws4 ) + H( 5)); \
                qt[ 5] = SPH_T32(ss0(Ws5 ) + H( 6)); \
                qt[ 6] = SPH_T32(ss1(Ws6 ) + H( 7)); \
                qt[ 7] = SPH_T32(ss2(Ws7 ) + H( 8)); \
                qt[ 8] = SPH_T32(ss3(Ws8 ) + H( 9)); \
                qt[ 9] = SPH_T32(ss4(Ws9 ) + H(10)); \
                qt[10] = SPH_T32(ss0(Ws10) + H(11)); \
                qt[11] = SPH_T32(ss1(Ws11) + H(12)); \
                qt[12] = SPH_T32(ss2(Ws12) + H(13)); \
                qt[13] = SPH_T32(ss3(Ws13) + H(14)); \
                qt[14] = SPH_T32(ss4(Ws14) + H(15)); \
                qt[15] = SPH_T32(ss0(Ws15) + H( 0)); \
        } while (0)

#define MAKE_Qbs   do { \
                qt[16] = expand1s(Qs, M, H, 16); \
                qt[17] = expand1s(Qs, M, H, 17); \
                qt[18] = expand2s(Qs, M, H, 18); \
                qt[19] = expand2s(Qs, M, H, 19); \
                qt[20] = expand2s(Qs, M, H, 20); \
                qt[21] = expand2s(Qs, M, H, 21); \
                qt[22] = expand2s(Qs, M, H, 22); \
                qt[23] = expand2s(Qs, M, H, 23); \
                qt[24] = expand2s(Qs, M, H, 24); \
                qt[25] = expand2s(Qs, M, H, 25); \
                qt[26] = expand2s(Qs, M, H, 26); \
                qt[27] = expand2s(Qs, M, H, 27); \
                qt[28] = expand2s(Qs, M, H, 28); \
                qt[29] = expand2s(Qs, M, H, 29); \
                qt[30] = expand2s(Qs, M, H, 30); \
                qt[31] = expand2s(Qs, M, H, 31); \
        } while (0)


#define MAKE_Qs   do { \
                MAKE_Qas; \
                MAKE_Qbs; \
        } while (0)

#define Qs(j)   (qt[j])

#define Wb0    MAKE_W(SPH_T64,  5, -,  7, +, 10, +, 13, +, 14)
#define Wb1    MAKE_W(SPH_T64,  6, -,  8, +, 11, +, 14, -, 15)
#define Wb2    MAKE_W(SPH_T64,  0, +,  7, +,  9, -, 12, +, 15)
#define Wb3    MAKE_W(SPH_T64,  0, -,  1, +,  8, -, 10, +, 13)
#define Wb4    MAKE_W(SPH_T64,  1, +,  2, +,  9, -, 11, -, 14)
#define Wb5    MAKE_W(SPH_T64,  3, -,  2, +, 10, -, 12, +, 15)
#define Wb6    MAKE_W(SPH_T64,  4, -,  0, -,  3, -, 11, +, 13)
#define Wb7    MAKE_W(SPH_T64,  1, -,  4, -,  5, -, 12, -, 14)
#define Wb8    MAKE_W(SPH_T64,  2, -,  5, -,  6, +, 13, -, 15)
#define Wb9    MAKE_W(SPH_T64,  0, -,  3, +,  6, -,  7, +, 14)
#define Wb10   MAKE_W(SPH_T64,  8, -,  1, -,  4, -,  7, +, 15)
#define Wb11   MAKE_W(SPH_T64,  8, -,  0, -,  2, -,  5, +,  9)
#define Wb12   MAKE_W(SPH_T64,  1, +,  3, -,  6, -,  9, +, 10)
#define Wb13   MAKE_W(SPH_T64,  2, +,  4, +,  7, +, 10, +, 11)
#define Wb14   MAKE_W(SPH_T64,  3, -,  5, +,  8, -, 11, -, 12)
#define Wb15   MAKE_W(SPH_T64, 12, -,  4, -,  6, -,  9, +, 13)

#define MAKE_Qab   do { \
                qt[ 0] = SPH_T64(sb0(Wb0 ) + H( 1)); \
                qt[ 1] = SPH_T64(sb1(Wb1 ) + H( 2)); \
                qt[ 2] = SPH_T64(sb2(Wb2 ) + H( 3)); \
                qt[ 3] = SPH_T64(sb3(Wb3 ) + H( 4)); \
                qt[ 4] = SPH_T64(sb4(Wb4 ) + H( 5)); \
                qt[ 5] = SPH_T64(sb0(Wb5 ) + H( 6)); \
                qt[ 6] = SPH_T64(sb1(Wb6 ) + H( 7)); \
                qt[ 7] = SPH_T64(sb2(Wb7 ) + H( 8)); \
                qt[ 8] = SPH_T64(sb3(Wb8 ) + H( 9)); \
                qt[ 9] = SPH_T64(sb4(Wb9 ) + H(10)); \
                qt[10] = SPH_T64(sb0(Wb10) + H(11)); \
                qt[11] = SPH_T64(sb1(Wb11) + H(12)); \
                qt[12] = SPH_T64(sb2(Wb12) + H(13)); \
                qt[13] = SPH_T64(sb3(Wb13) + H(14)); \
                qt[14] = SPH_T64(sb4(Wb14) + H(15)); \
                qt[15] = SPH_T64(sb0(Wb15) + H( 0)); \
        } while (0)

#define MAKE_Qbb   do { \
                qt[16] = expand1b(Qb, M, H, 16); \
                qt[17] = expand1b(Qb, M, H, 17); \
                qt[18] = expand2b(Qb, M, H, 18); \
                qt[19] = expand2b(Qb, M, H, 19); \
                qt[20] = expand2b(Qb, M, H, 20); \
                qt[21] = expand2b(Qb, M, H, 21); \
                qt[22] = expand2b(Qb, M, H, 22); \
                qt[23] = expand2b(Qb, M, H, 23); \
                qt[24] = expand2b(Qb, M, H, 24); \
                qt[25] = expand2b(Qb, M, H, 25); \
                qt[26] = expand2b(Qb, M, H, 26); \
                qt[27] = expand2b(Qb, M, H, 27); \
                qt[28] = expand2b(Qb, M, H, 28); \
                qt[29] = expand2b(Qb, M, H, 29); \
                qt[30] = expand2b(Qb, M, H, 30); \
                qt[31] = expand2b(Qb, M, H, 31); \
        } while (0)


#define MAKE_Qb   do { \
                MAKE_Qab; \
                MAKE_Qbb; \
        } while (0)

#define Qb(j)   (qt[j])


#define FOLD(type, mkQ, tt, rol, mf, qf, dhf)   do { \
                type qt[32], xl, xh; \
                mkQ; \
                xl = qf(16) ^ qf(17) ^ qf(18) ^ qf(19) \
                        ^ qf(20) ^ qf(21) ^ qf(22) ^ qf(23); \
                xh = xl ^ qf(24) ^ qf(25) ^ qf(26) ^ qf(27) \
                        ^ qf(28) ^ qf(29) ^ qf(30) ^ qf(31); \
                dhf( 0) = tt(((xh <<  5) ^ (qf(16) >>  5) ^ mf( 0)) \
                        + (xl ^ qf(24) ^ qf( 0))); \
                dhf( 1) = tt(((xh >>  7) ^ (qf(17) <<  8) ^ mf( 1)) \
                        + (xl ^ qf(25) ^ qf( 1))); \
                dhf( 2) = tt(((xh >>  5) ^ (qf(18) <<  5) ^ mf( 2)) \
                        + (xl ^ qf(26) ^ qf( 2))); \
                dhf( 3) = tt(((xh >>  1) ^ (qf(19) <<  5) ^ mf( 3)) \
                        + (xl ^ qf(27) ^ qf( 3))); \
                dhf( 4) = tt(((xh >>  3) ^ (qf(20) <<  0) ^ mf( 4)) \
                        + (xl ^ qf(28) ^ qf( 4))); \
                dhf( 5) = tt(((xh <<  6) ^ (qf(21) >>  6) ^ mf( 5)) \
                        + (xl ^ qf(29) ^ qf( 5))); \
                dhf( 6) = tt(((xh >>  4) ^ (qf(22) <<  6) ^ mf( 6)) \
                        + (xl ^ qf(30) ^ qf( 6))); \
                dhf( 7) = tt(((xh >> 11) ^ (qf(23) <<  2) ^ mf( 7)) \
                        + (xl ^ qf(31) ^ qf( 7))); \
                dhf( 8) = tt(rol(dhf(4),  9) + (xh ^ qf(24) ^ mf( 8)) \
                        + ((xl << 8) ^ qf(23) ^ qf( 8))); \
                dhf( 9) = tt(rol(dhf(5), 10) + (xh ^ qf(25) ^ mf( 9)) \
                        + ((xl >> 6) ^ qf(16) ^ qf( 9))); \
                dhf(10) = tt(rol(dhf(6), 11) + (xh ^ qf(26) ^ mf(10)) \
                        + ((xl << 6) ^ qf(17) ^ qf(10))); \
                dhf(11) = tt(rol(dhf(7), 12) + (xh ^ qf(27) ^ mf(11)) \
                        + ((xl << 4) ^ qf(18) ^ qf(11))); \
                dhf(12) = tt(rol(dhf(0), 13) + (xh ^ qf(28) ^ mf(12)) \
                        + ((xl >> 3) ^ qf(19) ^ qf(12))); \
                dhf(13) = tt(rol(dhf(1), 14) + (xh ^ qf(29) ^ mf(13)) \
                        + ((xl >> 4) ^ qf(20) ^ qf(13))); \
                dhf(14) = tt(rol(dhf(2), 15) + (xh ^ qf(30) ^ mf(14)) \
                        + ((xl >> 7) ^ qf(21) ^ qf(14))); \
                dhf(15) = tt(rol(dhf(3), 16) + (xh ^ qf(31) ^ mf(15)) \
                        + ((xl >> 2) ^ qf(22) ^ qf(15))); \
        } while (0)

#define FOLDs   FOLD(sph_u32, MAKE_Qs, SPH_T32, SPH_ROTL32, M, Qs, dH)
#define FOLDb   FOLD(sph_u64, MAKE_Qb, SPH_T64, SPH_ROTL64, M, Qb, dH)


static void
compress_small(const unsigned char *data, const sph_u32 h[16], sph_u32 dh[16])
{
#define M(x)    sph_dec32le_aligned(data + 4 * (x))
#define H(x)    (h[x])
#define dH(x)   (dh[x])

        FOLDs;

#undef M
#undef H
#undef dH
}

static const sph_u32 final_s[16] = {
        SPH_C32(0xaaaaaaa0), SPH_C32(0xaaaaaaa1), SPH_C32(0xaaaaaaa2),
        SPH_C32(0xaaaaaaa3), SPH_C32(0xaaaaaaa4), SPH_C32(0xaaaaaaa5),
        SPH_C32(0xaaaaaaa6), SPH_C32(0xaaaaaaa7), SPH_C32(0xaaaaaaa8),
        SPH_C32(0xaaaaaaa9), SPH_C32(0xaaaaaaaa), SPH_C32(0xaaaaaaab),
        SPH_C32(0xaaaaaaac), SPH_C32(0xaaaaaaad), SPH_C32(0xaaaaaaae),
        SPH_C32(0xaaaaaaaf)
};

static void
bmw32_init(sph_bmw_small_context *sc, const sph_u32 *iv)
{
        memcpy(sc->H, iv, sizeof sc->H);
        sc->ptr = 0;
        sc->bit_count = 0;
}

static void
bmw32(sph_bmw_small_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;
        sph_u32 htmp[16];
        sph_u32 *h1, *h2;

        sc->bit_count += (sph_u64)len << 3;
        buf = sc->buf;
        ptr = sc->ptr;
        h1 = sc->H;
        h2 = htmp;
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                data = (const unsigned char *)data + clen;
                len -= clen;
                ptr += clen;
                if (ptr == sizeof sc->buf) {
                        sph_u32 *ht;

                        compress_small(buf, h1, h2);
                        ht = h1;
                        h1 = h2;
                        h2 = ht;
                        ptr = 0;
                }
        }
        sc->ptr = ptr;
        if (h1 != sc->H)
                memcpy(sc->H, h1, sizeof sc->H);
}

static void
bmw32_close(sph_bmw_small_context *sc, unsigned ub, unsigned n,
    unsigned char *dst, size_t out_size_w32)
{
        unsigned char *buf, *out;
        size_t ptr, u, v;
        unsigned z;
        sph_u32 h1[16], h2[16], *h;

        buf = sc->buf;
        ptr = sc->ptr;
        z = 0x80 >> n;
        buf[ptr ++] = ((ub & -z) | z) & 0xFF;
        h = sc->H;
        if (ptr > (sizeof sc->buf) - 8) {
                memset(buf + ptr, 0, (sizeof sc->buf) - ptr);
                compress_small(buf, h, h1);
                ptr = 0;
                h = h1;
        }
        memset(buf + ptr, 0, (sizeof sc->buf) - 8 - ptr);
        sph_enc64le_aligned(buf + (sizeof sc->buf) - 8,
                SPH_T64(sc->bit_count + n));
        compress_small(buf, h, h2);
        for (u = 0; u < 16; u ++)
                sph_enc32le_aligned(buf + 4 * u, h2[u]);
        compress_small(buf, final_s, h1);
        out = dst;
        for (u = 0, v = 16 - out_size_w32; u < out_size_w32; u ++, v ++)
                sph_enc32le(out + 4 * u, h1[v]);
}


static void
compress_big(const unsigned char *data, const sph_u64 h[16], sph_u64 dh[16])
{
#define M(x)    sph_dec64le_aligned(data + 8 * (x))
#define H(x)    (h[x])
#define dH(x)   (dh[x])

        FOLDb;

#undef M
#undef H
#undef dH
}

static const sph_u64 final_b[16] = {
        SPH_C64(0xaaaaaaaaaaaaaaa0), SPH_C64(0xaaaaaaaaaaaaaaa1),
        SPH_C64(0xaaaaaaaaaaaaaaa2), SPH_C64(0xaaaaaaaaaaaaaaa3),
        SPH_C64(0xaaaaaaaaaaaaaaa4), SPH_C64(0xaaaaaaaaaaaaaaa5),
        SPH_C64(0xaaaaaaaaaaaaaaa6), SPH_C64(0xaaaaaaaaaaaaaaa7),
        SPH_C64(0xaaaaaaaaaaaaaaa8), SPH_C64(0xaaaaaaaaaaaaaaa9),
        SPH_C64(0xaaaaaaaaaaaaaaaa), SPH_C64(0xaaaaaaaaaaaaaaab),
        SPH_C64(0xaaaaaaaaaaaaaaac), SPH_C64(0xaaaaaaaaaaaaaaad),
        SPH_C64(0xaaaaaaaaaaaaaaae), SPH_C64(0xaaaaaaaaaaaaaaaf)
};

static void
bmw64_init(sph_bmw_big_context *sc, const sph_u64 *iv)
{
        memcpy(sc->H, iv, sizeof sc->H);
        sc->ptr = 0;
        sc->bit_count = 0;
}

static void
bmw64(sph_bmw_big_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;
        sph_u64 htmp[16];
        sph_u64 *h1, *h2;

        sc->bit_count += (sph_u64)len << 3;
        buf = sc->buf;
        ptr = sc->ptr;
        h1 = sc->H;
        h2 = htmp;
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                data = (const unsigned char *)data + clen;
                len -= clen;
                ptr += clen;
                if (ptr == sizeof sc->buf) {
                        sph_u64 *ht;

                        compress_big(buf, h1, h2);
                        ht = h1;
                        h1 = h2;
                        h2 = ht;
                        ptr = 0;
                }
        }
        sc->ptr = ptr;
        if (h1 != sc->H)
                memcpy(sc->H, h1, sizeof sc->H);
}

static void
bmw64_close(sph_bmw_big_context *sc, unsigned ub, unsigned n,
    unsigned char *dst, size_t out_size_w64)
{
        unsigned char *buf, *out;
        size_t ptr, u, v;
        unsigned z;
        sph_u64 h1[16], h2[16], *h;

        buf = sc->buf;
        ptr = sc->ptr;
        z = 0x80 >> n;
        buf[ptr ++] = ((ub & -z) | z) & 0xFF;
        h = sc->H;
        if (ptr > (sizeof sc->buf) - 8) {
                memset(buf + ptr, 0, (sizeof sc->buf) - ptr);
                compress_big(buf, h, h1);
                ptr = 0;
                h = h1;
        }
        memset(buf + ptr, 0, (sizeof sc->buf) - 8 - ptr);
        sph_enc64le_aligned(buf + (sizeof sc->buf) - 8,
                SPH_T64(sc->bit_count + n));
        compress_big(buf, h, h2);
        for (u = 0; u < 16; u ++)
                sph_enc64le_aligned(buf + 8 * u, h2[u]);
        compress_big(buf, final_b, h1);
        out = dst;
        for (u = 0, v = 16 - out_size_w64; u < out_size_w64; u ++, v ++)
                sph_enc64le(out + 8 * u, h1[v]);
}


void
sph_bmw224_init(sph_bmw_small_context *cc)
{
        bmw32_init(cc, bmw_IV224);
}

void
sph_bmw224(sph_bmw_small_context *cc, const unsigned char *data, size_t len)
{
        bmw32(cc, data, len);
}

void
sph_bmw224_close(sph_bmw_small_context *cc, unsigned char *dst)
{
        sph_bmw224_addbits_and_close(cc, 0, 0, dst);
}

void
sph_bmw224_addbits_and_close(sph_bmw_small_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        bmw32_close(cc, ub, n, dst, 7);
        sph_bmw224_init(cc);
}

void
sph_bmw256_init(sph_bmw_small_context *cc)
{
        bmw32_init(cc, bmw_IV256);
}

void
sph_bmw256(sph_bmw_small_context *cc, const unsigned char *data, size_t len)
{
        bmw32(cc, data, len);
}

void
sph_bmw256_close(sph_bmw_small_context *cc, unsigned char *dst)
{
        sph_bmw256_addbits_and_close(cc, 0, 0, dst);
}

void
sph_bmw256_addbits_and_close(sph_bmw_small_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        bmw32_close(cc, ub, n, dst, 8);
        sph_bmw256_init(cc);
}

void
sph_bmw384_init(sph_bmw_big_context *cc)
{
        bmw64_init(cc, bmw_IV384);
}

void
sph_bmw384(sph_bmw_big_context *cc, const unsigned char *data, size_t len)
{
        bmw64(cc, data, len);
}

void
sph_bmw384_close(sph_bmw_big_context *cc, unsigned char *dst)
{
        sph_bmw384_addbits_and_close(cc, 0, 0, dst);
}

void
sph_bmw384_addbits_and_close(sph_bmw_big_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        bmw64_close(cc, ub, n, dst, 6);
        sph_bmw384_init(cc);
}

void
sph_bmw512_init(sph_bmw_big_context *cc)
{
        bmw64_init(cc, bmw_IV512);
}

void
sph_bmw512(sph_bmw_big_context *cc, const unsigned char *data, size_t len)
{
        bmw64(cc, data, len);
}

void
sph_bmw512_close(sph_bmw_big_context *cc, unsigned char *dst)
{
        sph_bmw512_addbits_and_close(cc, 0, 0, dst);
}

void
sph_bmw512_addbits_and_close(sph_bmw_big_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        bmw64_close(cc, ub, n, dst, 8);
        sph_bmw512_init(cc);
}










#define SPH_SIZE_cubehash224   224
#define SPH_SIZE_cubehash256   256
#define SPH_SIZE_cubehash384   384
#define SPH_SIZE_cubehash512   512

typedef struct {
        unsigned char buf[32];    /* first field, for alignment */
        size_t ptr;
        sph_u32 state[32];
} sph_cubehash_context;

void
sph_cubehash224_init(sph_cubehash_context *cc);
void
sph_cubehash224(sph_cubehash_context *cc, const unsigned char *data, size_t len);
void
sph_cubehash224_close(sph_cubehash_context *cc, unsigned char *dst);
void
sph_cubehash224_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_cubehash256_init(sph_cubehash_context *cc);
void
sph_cubehash256(sph_cubehash_context *cc, const unsigned char *data, size_t len);
void
sph_cubehash256_close(sph_cubehash_context *cc, unsigned char *dst);
void
sph_cubehash256_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_cubehash384_init(sph_cubehash_context *cc);
void
sph_cubehash384(sph_cubehash_context *cc, const unsigned char *data, size_t len);
void
sph_cubehash384_close(sph_cubehash_context *cc, unsigned char *dst);
void
sph_cubehash384_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_cubehash512_init(sph_cubehash_context *cc);
void
sph_cubehash512(sph_cubehash_context *cc, const unsigned char *data, size_t len);
void
sph_cubehash512_close(sph_cubehash_context *cc, unsigned char *dst);
void
sph_cubehash512_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst);






#define SPH_CUBEHASH_UNROLL   0
#define SPH_CUBEHASH_NOCOPY   0


static const sph_u32 cubehash_IV224[] = {
        SPH_C32(0xB0FC8217), SPH_C32(0x1BEE1A90), SPH_C32(0x829E1A22),
        SPH_C32(0x6362C342), SPH_C32(0x24D91C30), SPH_C32(0x03A7AA24),
        SPH_C32(0xA63721C8), SPH_C32(0x85B0E2EF), SPH_C32(0xF35D13F3),
        SPH_C32(0x41DA807D), SPH_C32(0x21A70CA6), SPH_C32(0x1F4E9774),
        SPH_C32(0xB3E1C932), SPH_C32(0xEB0A79A8), SPH_C32(0xCDDAAA66),
        SPH_C32(0xE2F6ECAA), SPH_C32(0x0A713362), SPH_C32(0xAA3080E0),
        SPH_C32(0xD8F23A32), SPH_C32(0xCEF15E28), SPH_C32(0xDB086314),
        SPH_C32(0x7F709DF7), SPH_C32(0xACD228A4), SPH_C32(0x704D6ECE),
        SPH_C32(0xAA3EC95F), SPH_C32(0xE387C214), SPH_C32(0x3A6445FF),
        SPH_C32(0x9CAB81C3), SPH_C32(0xC73D4B98), SPH_C32(0xD277AEBE),
        SPH_C32(0xFD20151C), SPH_C32(0x00CB573E)
};

static const sph_u32 cubehash_IV256[] = {
        SPH_C32(0xEA2BD4B4), SPH_C32(0xCCD6F29F), SPH_C32(0x63117E71),
        SPH_C32(0x35481EAE), SPH_C32(0x22512D5B), SPH_C32(0xE5D94E63),
        SPH_C32(0x7E624131), SPH_C32(0xF4CC12BE), SPH_C32(0xC2D0B696),
        SPH_C32(0x42AF2070), SPH_C32(0xD0720C35), SPH_C32(0x3361DA8C),
        SPH_C32(0x28CCECA4), SPH_C32(0x8EF8AD83), SPH_C32(0x4680AC00),
        SPH_C32(0x40E5FBAB), SPH_C32(0xD89041C3), SPH_C32(0x6107FBD5),
        SPH_C32(0x6C859D41), SPH_C32(0xF0B26679), SPH_C32(0x09392549),
        SPH_C32(0x5FA25603), SPH_C32(0x65C892FD), SPH_C32(0x93CB6285),
        SPH_C32(0x2AF2B5AE), SPH_C32(0x9E4B4E60), SPH_C32(0x774ABFDD),
        SPH_C32(0x85254725), SPH_C32(0x15815AEB), SPH_C32(0x4AB6AAD6),
        SPH_C32(0x9CDAF8AF), SPH_C32(0xD6032C0A)
};

static const sph_u32 cubehash_IV384[] = {
        SPH_C32(0xE623087E), SPH_C32(0x04C00C87), SPH_C32(0x5EF46453),
        SPH_C32(0x69524B13), SPH_C32(0x1A05C7A9), SPH_C32(0x3528DF88),
        SPH_C32(0x6BDD01B5), SPH_C32(0x5057B792), SPH_C32(0x6AA7A922),
        SPH_C32(0x649C7EEE), SPH_C32(0xF426309F), SPH_C32(0xCB629052),
        SPH_C32(0xFC8E20ED), SPH_C32(0xB3482BAB), SPH_C32(0xF89E5E7E),
        SPH_C32(0xD83D4DE4), SPH_C32(0x44BFC10D), SPH_C32(0x5FC1E63D),
        SPH_C32(0x2104E6CB), SPH_C32(0x17958F7F), SPH_C32(0xDBEAEF70),
        SPH_C32(0xB4B97E1E), SPH_C32(0x32C195F6), SPH_C32(0x6184A8E4),
        SPH_C32(0x796C2543), SPH_C32(0x23DE176D), SPH_C32(0xD33BBAEC),
        SPH_C32(0x0C12E5D2), SPH_C32(0x4EB95A7B), SPH_C32(0x2D18BA01),
        SPH_C32(0x04EE475F), SPH_C32(0x1FC5F22E)
};

static const sph_u32 cubehash_IV512[] = {
        SPH_C32(0x2AEA2A61), SPH_C32(0x50F494D4), SPH_C32(0x2D538B8B),
        SPH_C32(0x4167D83E), SPH_C32(0x3FEE2313), SPH_C32(0xC701CF8C),
        SPH_C32(0xCC39968E), SPH_C32(0x50AC5695), SPH_C32(0x4D42C787),
        SPH_C32(0xA647A8B3), SPH_C32(0x97CF0BEF), SPH_C32(0x825B4537),
        SPH_C32(0xEEF864D2), SPH_C32(0xF22090C4), SPH_C32(0xD0E5CD33),
        SPH_C32(0xA23911AE), SPH_C32(0xFCD398D9), SPH_C32(0x148FE485),
        SPH_C32(0x1B017BEF), SPH_C32(0xB6444532), SPH_C32(0x6A536159),
        SPH_C32(0x2FF5781C), SPH_C32(0x91FA7934), SPH_C32(0x0DBADEA9),
        SPH_C32(0xD65C8A2B), SPH_C32(0xA5A70E75), SPH_C32(0xB1C62456),
        SPH_C32(0xBC796576), SPH_C32(0x1921C8F7), SPH_C32(0xE7989AF1),
        SPH_C32(0x7795D246), SPH_C32(0xD43E3B44)
};

#define T32      SPH_T32
#define ROTL32   SPH_ROTL32

#define cubehash_DECL_STATE \
        sph_u32 x0, x1, x2, x3, x4, x5, x6, x7; \
        sph_u32 x8, x9, xa, xb, xc, xd, xe, xf; \
        sph_u32 xg, xh, xi, xj, xk, xl, xm, xn; \
        sph_u32 xo, xp, xq, xr, xs, xt, xu, xv;

#define cubehash_READ_STATE(cc)   do { \
                x0 = (cc)->state[ 0]; \
                x1 = (cc)->state[ 1]; \
                x2 = (cc)->state[ 2]; \
                x3 = (cc)->state[ 3]; \
                x4 = (cc)->state[ 4]; \
                x5 = (cc)->state[ 5]; \
                x6 = (cc)->state[ 6]; \
                x7 = (cc)->state[ 7]; \
                x8 = (cc)->state[ 8]; \
                x9 = (cc)->state[ 9]; \
                xa = (cc)->state[10]; \
                xb = (cc)->state[11]; \
                xc = (cc)->state[12]; \
                xd = (cc)->state[13]; \
                xe = (cc)->state[14]; \
                xf = (cc)->state[15]; \
                xg = (cc)->state[16]; \
                xh = (cc)->state[17]; \
                xi = (cc)->state[18]; \
                xj = (cc)->state[19]; \
                xk = (cc)->state[20]; \
                xl = (cc)->state[21]; \
                xm = (cc)->state[22]; \
                xn = (cc)->state[23]; \
                xo = (cc)->state[24]; \
                xp = (cc)->state[25]; \
                xq = (cc)->state[26]; \
                xr = (cc)->state[27]; \
                xs = (cc)->state[28]; \
                xt = (cc)->state[29]; \
                xu = (cc)->state[30]; \
                xv = (cc)->state[31]; \
        } while (0)

#define cubehash_WRITE_STATE(cc)   do { \
                (cc)->state[ 0] = x0; \
                (cc)->state[ 1] = x1; \
                (cc)->state[ 2] = x2; \
                (cc)->state[ 3] = x3; \
                (cc)->state[ 4] = x4; \
                (cc)->state[ 5] = x5; \
                (cc)->state[ 6] = x6; \
                (cc)->state[ 7] = x7; \
                (cc)->state[ 8] = x8; \
                (cc)->state[ 9] = x9; \
                (cc)->state[10] = xa; \
                (cc)->state[11] = xb; \
                (cc)->state[12] = xc; \
                (cc)->state[13] = xd; \
                (cc)->state[14] = xe; \
                (cc)->state[15] = xf; \
                (cc)->state[16] = xg; \
                (cc)->state[17] = xh; \
                (cc)->state[18] = xi; \
                (cc)->state[19] = xj; \
                (cc)->state[20] = xk; \
                (cc)->state[21] = xl; \
                (cc)->state[22] = xm; \
                (cc)->state[23] = xn; \
                (cc)->state[24] = xo; \
                (cc)->state[25] = xp; \
                (cc)->state[26] = xq; \
                (cc)->state[27] = xr; \
                (cc)->state[28] = xs; \
                (cc)->state[29] = xt; \
                (cc)->state[30] = xu; \
                (cc)->state[31] = xv; \
        } while (0)

#define INPUT_BLOCK   do { \
                x0 ^= sph_dec32le_aligned(buf +  0); \
                x1 ^= sph_dec32le_aligned(buf +  4); \
                x2 ^= sph_dec32le_aligned(buf +  8); \
                x3 ^= sph_dec32le_aligned(buf + 12); \
                x4 ^= sph_dec32le_aligned(buf + 16); \
                x5 ^= sph_dec32le_aligned(buf + 20); \
                x6 ^= sph_dec32le_aligned(buf + 24); \
                x7 ^= sph_dec32le_aligned(buf + 28); \
        } while (0)

#define ROUND_EVEN   do { \
                xg = T32(x0 + xg); \
                x0 = ROTL32(x0, 7); \
                xh = T32(x1 + xh); \
                x1 = ROTL32(x1, 7); \
                xi = T32(x2 + xi); \
                x2 = ROTL32(x2, 7); \
                xj = T32(x3 + xj); \
                x3 = ROTL32(x3, 7); \
                xk = T32(x4 + xk); \
                x4 = ROTL32(x4, 7); \
                xl = T32(x5 + xl); \
                x5 = ROTL32(x5, 7); \
                xm = T32(x6 + xm); \
                x6 = ROTL32(x6, 7); \
                xn = T32(x7 + xn); \
                x7 = ROTL32(x7, 7); \
                xo = T32(x8 + xo); \
                x8 = ROTL32(x8, 7); \
                xp = T32(x9 + xp); \
                x9 = ROTL32(x9, 7); \
                xq = T32(xa + xq); \
                xa = ROTL32(xa, 7); \
                xr = T32(xb + xr); \
                xb = ROTL32(xb, 7); \
                xs = T32(xc + xs); \
                xc = ROTL32(xc, 7); \
                xt = T32(xd + xt); \
                xd = ROTL32(xd, 7); \
                xu = T32(xe + xu); \
                xe = ROTL32(xe, 7); \
                xv = T32(xf + xv); \
                xf = ROTL32(xf, 7); \
                x8 ^= xg; \
                x9 ^= xh; \
                xa ^= xi; \
                xb ^= xj; \
                xc ^= xk; \
                xd ^= xl; \
                xe ^= xm; \
                xf ^= xn; \
                x0 ^= xo; \
                x1 ^= xp; \
                x2 ^= xq; \
                x3 ^= xr; \
                x4 ^= xs; \
                x5 ^= xt; \
                x6 ^= xu; \
                x7 ^= xv; \
                xi = T32(x8 + xi); \
                x8 = ROTL32(x8, 11); \
                xj = T32(x9 + xj); \
                x9 = ROTL32(x9, 11); \
                xg = T32(xa + xg); \
                xa = ROTL32(xa, 11); \
                xh = T32(xb + xh); \
                xb = ROTL32(xb, 11); \
                xm = T32(xc + xm); \
                xc = ROTL32(xc, 11); \
                xn = T32(xd + xn); \
                xd = ROTL32(xd, 11); \
                xk = T32(xe + xk); \
                xe = ROTL32(xe, 11); \
                xl = T32(xf + xl); \
                xf = ROTL32(xf, 11); \
                xq = T32(x0 + xq); \
                x0 = ROTL32(x0, 11); \
                xr = T32(x1 + xr); \
                x1 = ROTL32(x1, 11); \
                xo = T32(x2 + xo); \
                x2 = ROTL32(x2, 11); \
                xp = T32(x3 + xp); \
                x3 = ROTL32(x3, 11); \
                xu = T32(x4 + xu); \
                x4 = ROTL32(x4, 11); \
                xv = T32(x5 + xv); \
                x5 = ROTL32(x5, 11); \
                xs = T32(x6 + xs); \
                x6 = ROTL32(x6, 11); \
                xt = T32(x7 + xt); \
                x7 = ROTL32(x7, 11); \
                xc ^= xi; \
                xd ^= xj; \
                xe ^= xg; \
                xf ^= xh; \
                x8 ^= xm; \
                x9 ^= xn; \
                xa ^= xk; \
                xb ^= xl; \
                x4 ^= xq; \
                x5 ^= xr; \
                x6 ^= xo; \
                x7 ^= xp; \
                x0 ^= xu; \
                x1 ^= xv; \
                x2 ^= xs; \
                x3 ^= xt; \
        } while (0)

#define ROUND_ODD   do { \
                xj = T32(xc + xj); \
                xc = ROTL32(xc, 7); \
                xi = T32(xd + xi); \
                xd = ROTL32(xd, 7); \
                xh = T32(xe + xh); \
                xe = ROTL32(xe, 7); \
                xg = T32(xf + xg); \
                xf = ROTL32(xf, 7); \
                xn = T32(x8 + xn); \
                x8 = ROTL32(x8, 7); \
                xm = T32(x9 + xm); \
                x9 = ROTL32(x9, 7); \
                xl = T32(xa + xl); \
                xa = ROTL32(xa, 7); \
                xk = T32(xb + xk); \
                xb = ROTL32(xb, 7); \
                xr = T32(x4 + xr); \
                x4 = ROTL32(x4, 7); \
                xq = T32(x5 + xq); \
                x5 = ROTL32(x5, 7); \
                xp = T32(x6 + xp); \
                x6 = ROTL32(x6, 7); \
                xo = T32(x7 + xo); \
                x7 = ROTL32(x7, 7); \
                xv = T32(x0 + xv); \
                x0 = ROTL32(x0, 7); \
                xu = T32(x1 + xu); \
                x1 = ROTL32(x1, 7); \
                xt = T32(x2 + xt); \
                x2 = ROTL32(x2, 7); \
                xs = T32(x3 + xs); \
                x3 = ROTL32(x3, 7); \
                x4 ^= xj; \
                x5 ^= xi; \
                x6 ^= xh; \
                x7 ^= xg; \
                x0 ^= xn; \
                x1 ^= xm; \
                x2 ^= xl; \
                x3 ^= xk; \
                xc ^= xr; \
                xd ^= xq; \
                xe ^= xp; \
                xf ^= xo; \
                x8 ^= xv; \
                x9 ^= xu; \
                xa ^= xt; \
                xb ^= xs; \
                xh = T32(x4 + xh); \
                x4 = ROTL32(x4, 11); \
                xg = T32(x5 + xg); \
                x5 = ROTL32(x5, 11); \
                xj = T32(x6 + xj); \
                x6 = ROTL32(x6, 11); \
                xi = T32(x7 + xi); \
                x7 = ROTL32(x7, 11); \
                xl = T32(x0 + xl); \
                x0 = ROTL32(x0, 11); \
                xk = T32(x1 + xk); \
                x1 = ROTL32(x1, 11); \
                xn = T32(x2 + xn); \
                x2 = ROTL32(x2, 11); \
                xm = T32(x3 + xm); \
                x3 = ROTL32(x3, 11); \
                xp = T32(xc + xp); \
                xc = ROTL32(xc, 11); \
                xo = T32(xd + xo); \
                xd = ROTL32(xd, 11); \
                xr = T32(xe + xr); \
                xe = ROTL32(xe, 11); \
                xq = T32(xf + xq); \
                xf = ROTL32(xf, 11); \
                xt = T32(x8 + xt); \
                x8 = ROTL32(x8, 11); \
                xs = T32(x9 + xs); \
                x9 = ROTL32(x9, 11); \
                xv = T32(xa + xv); \
                xa = ROTL32(xa, 11); \
                xu = T32(xb + xu); \
                xb = ROTL32(xb, 11); \
                x0 ^= xh; \
                x1 ^= xg; \
                x2 ^= xj; \
                x3 ^= xi; \
                x4 ^= xl; \
                x5 ^= xk; \
                x6 ^= xn; \
                x7 ^= xm; \
                x8 ^= xp; \
                x9 ^= xo; \
                xa ^= xr; \
                xb ^= xq; \
                xc ^= xt; \
                xd ^= xs; \
                xe ^= xv; \
                xf ^= xu; \
        } while (0)


#define SIXTEEN_ROUNDS   do { \
                ROUND_EVEN; \
                ROUND_ODD; \
                ROUND_EVEN; \
                ROUND_ODD; \
                ROUND_EVEN; \
                ROUND_ODD; \
                ROUND_EVEN; \
                ROUND_ODD; \
                ROUND_EVEN; \
                ROUND_ODD; \
                ROUND_EVEN; \
                ROUND_ODD; \
                ROUND_EVEN; \
                ROUND_ODD; \
                ROUND_EVEN; \
                ROUND_ODD; \
        } while (0)


static void
cubehash_init(sph_cubehash_context *sc, const sph_u32 *iv)
{
        memcpy(sc->state, iv, sizeof sc->state);
        sc->ptr = 0;
}

static void
cubehash_core(sph_cubehash_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;
        cubehash_DECL_STATE

        buf = sc->buf;
        ptr = sc->ptr;
        if (len < (sizeof sc->buf) - ptr) {
                memcpy(buf + ptr, data, len);
                ptr += len;
                sc->ptr = ptr;
                return;
        }

        cubehash_READ_STATE(sc);
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                ptr += clen;
                data = (const unsigned char *)data + clen;
                len -= clen;
                if (ptr == sizeof sc->buf) {
                        INPUT_BLOCK;
                        SIXTEEN_ROUNDS;
                        ptr = 0;
                }
        }
        cubehash_WRITE_STATE(sc);
        sc->ptr = ptr;
}

static void
cubehash_close(sph_cubehash_context *sc, unsigned ub, unsigned n,
    unsigned char *dst, size_t out_size_w32)
{
        unsigned char *buf, *out;
        size_t ptr;
        unsigned z;
        int i;
        cubehash_DECL_STATE

        buf = sc->buf;
        ptr = sc->ptr;
        z = 0x80 >> n;
        buf[ptr ++] = ((ub & -z) | z) & 0xFF;
        memset(buf + ptr, 0, (sizeof sc->buf) - ptr);
        cubehash_READ_STATE(sc);
        INPUT_BLOCK;
        for (i = 0; i < 11; i ++) {
                SIXTEEN_ROUNDS;
                if (i == 0)
                        xv ^= SPH_C32(1);
        }
        cubehash_WRITE_STATE(sc);
        out = dst;
        for (z = 0; z < out_size_w32; z ++)
                sph_enc32le(out + (z << 2), sc->state[z]);
}

void
sph_cubehash224_init(sph_cubehash_context *cc)
{
        cubehash_init(cc, cubehash_IV224);
}

void
sph_cubehash224(sph_cubehash_context *cc, const unsigned char *data, size_t len)
{
        cubehash_core(cc, data, len);
}

void
sph_cubehash224_close(sph_cubehash_context *cc, unsigned char *dst)
{
        sph_cubehash224_addbits_and_close(cc, 0, 0, dst);
}

void
sph_cubehash224_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        cubehash_close(cc, ub, n, dst, 7);
        sph_cubehash224_init(cc);
}

void
sph_cubehash256_init(sph_cubehash_context *cc)
{
        cubehash_init(cc, cubehash_IV256);
}

void
sph_cubehash256(sph_cubehash_context *cc, const unsigned char *data, size_t len)
{
        cubehash_core(cc, data, len);
}

void
sph_cubehash256_close(sph_cubehash_context *cc, unsigned char *dst)
{
        sph_cubehash256_addbits_and_close(cc, 0, 0, dst);
}

void
sph_cubehash256_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        cubehash_close(cc, ub, n, dst, 8);
        sph_cubehash256_init(cc);
}

void
sph_cubehash384_init(sph_cubehash_context *cc)
{
        cubehash_init(cc, cubehash_IV384);
}

void
sph_cubehash384(sph_cubehash_context *cc, const unsigned char *data, size_t len)
{
        cubehash_core(cc, data, len);
}

void
sph_cubehash384_close(sph_cubehash_context *cc, unsigned char *dst)
{
        sph_cubehash384_addbits_and_close(cc, 0, 0, dst);
}

void
sph_cubehash384_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        cubehash_close(cc, ub, n, dst, 12);
        sph_cubehash384_init(cc);
}

void
sph_cubehash512_init(sph_cubehash_context *cc)
{
        cubehash_init(cc, cubehash_IV512);
}

void
sph_cubehash512(sph_cubehash_context *cc, const unsigned char *data, size_t len)
{
        cubehash_core(cc, data, len);
}

void
sph_cubehash512_close(sph_cubehash_context *cc, unsigned char *dst)
{
        sph_cubehash512_addbits_and_close(cc, 0, 0, dst);
}

void
sph_cubehash512_addbits_and_close(sph_cubehash_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        cubehash_close(cc, ub, n, dst, 16);
        sph_cubehash512_init(cc);
}







#define SPH_SIZE_fugue224   224
#define SPH_SIZE_fugue256   256
#define SPH_SIZE_fugue384   384
#define SPH_SIZE_fugue512   512

typedef struct {
        sph_u32 partial;
        unsigned partial_len;
        unsigned round_shift;
        sph_u32 S[36];
        sph_u64 bit_count;
} sph_fugue_context;

void
sph_fugue224_init(sph_fugue_context *cc);
void
sph_fugue224(sph_fugue_context *cc, const unsigned char *data, size_t len);
void
sph_fugue224_close(sph_fugue_context *cc, unsigned char *dst);
void
sph_fugue224_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_fugue256_init(sph_fugue_context *cc);
void
sph_fugue256(sph_fugue_context *cc, const unsigned char *data, size_t len);
void
sph_fugue256_close(sph_fugue_context *cc, unsigned char *dst);
void
sph_fugue256_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_fugue384_init(sph_fugue_context *cc);
void
sph_fugue384(sph_fugue_context *cc, const unsigned char *data, size_t len);
void
sph_fugue384_close(sph_fugue_context *cc, unsigned char *dst);
void
sph_fugue384_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_fugue512_init(sph_fugue_context *cc);
void
sph_fugue512(sph_fugue_context *cc, const unsigned char *data, size_t len);
void
sph_fugue512_close(sph_fugue_context *cc, unsigned char *dst);
void
sph_fugue512_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst);








static const sph_u32 fugue_IV224[] = {
        SPH_C32(0xf4c9120d), SPH_C32(0x6286f757), SPH_C32(0xee39e01c),
        SPH_C32(0xe074e3cb), SPH_C32(0xa1127c62), SPH_C32(0x9a43d215),
        SPH_C32(0xbd8d679a)
};

static const sph_u32 fugue_IV256[] = {
        SPH_C32(0xe952bdde), SPH_C32(0x6671135f), SPH_C32(0xe0d4f668),
        SPH_C32(0xd2b0b594), SPH_C32(0xf96c621d), SPH_C32(0xfbf929de),
        SPH_C32(0x9149e899), SPH_C32(0x34f8c248)
};

static const sph_u32 fugue_IV384[] = {
        SPH_C32(0xaa61ec0d), SPH_C32(0x31252e1f), SPH_C32(0xa01db4c7),
        SPH_C32(0x00600985), SPH_C32(0x215ef44a), SPH_C32(0x741b5e9c),
        SPH_C32(0xfa693e9a), SPH_C32(0x473eb040), SPH_C32(0xe502ae8a),
        SPH_C32(0xa99c25e0), SPH_C32(0xbc95517c), SPH_C32(0x5c1095a1)
};

static const sph_u32 fugue_IV512[] = {
        SPH_C32(0x8807a57e), SPH_C32(0xe616af75), SPH_C32(0xc5d3e4db),
        SPH_C32(0xac9ab027), SPH_C32(0xd915f117), SPH_C32(0xb6eecc54),
        SPH_C32(0x06e8020b), SPH_C32(0x4a92efd1), SPH_C32(0xaac6e2c9),
        SPH_C32(0xddb21398), SPH_C32(0xcae65838), SPH_C32(0x437f203f),
        SPH_C32(0x25ea78e7), SPH_C32(0x951fddd6), SPH_C32(0xda6ed11d),
        SPH_C32(0xe13e3567)
};

static const sph_u32 mixtab0[] = {
        SPH_C32(0x63633297), SPH_C32(0x7c7c6feb), SPH_C32(0x77775ec7),
        SPH_C32(0x7b7b7af7), SPH_C32(0xf2f2e8e5), SPH_C32(0x6b6b0ab7),
        SPH_C32(0x6f6f16a7), SPH_C32(0xc5c56d39), SPH_C32(0x303090c0),
        SPH_C32(0x01010704), SPH_C32(0x67672e87), SPH_C32(0x2b2bd1ac),
        SPH_C32(0xfefeccd5), SPH_C32(0xd7d71371), SPH_C32(0xabab7c9a),
        SPH_C32(0x767659c3), SPH_C32(0xcaca4005), SPH_C32(0x8282a33e),
        SPH_C32(0xc9c94909), SPH_C32(0x7d7d68ef), SPH_C32(0xfafad0c5),
        SPH_C32(0x5959947f), SPH_C32(0x4747ce07), SPH_C32(0xf0f0e6ed),
        SPH_C32(0xadad6e82), SPH_C32(0xd4d41a7d), SPH_C32(0xa2a243be),
        SPH_C32(0xafaf608a), SPH_C32(0x9c9cf946), SPH_C32(0xa4a451a6),
        SPH_C32(0x727245d3), SPH_C32(0xc0c0762d), SPH_C32(0xb7b728ea),
        SPH_C32(0xfdfdc5d9), SPH_C32(0x9393d47a), SPH_C32(0x2626f298),
        SPH_C32(0x363682d8), SPH_C32(0x3f3fbdfc), SPH_C32(0xf7f7f3f1),
        SPH_C32(0xcccc521d), SPH_C32(0x34348cd0), SPH_C32(0xa5a556a2),
        SPH_C32(0xe5e58db9), SPH_C32(0xf1f1e1e9), SPH_C32(0x71714cdf),
        SPH_C32(0xd8d83e4d), SPH_C32(0x313197c4), SPH_C32(0x15156b54),
        SPH_C32(0x04041c10), SPH_C32(0xc7c76331), SPH_C32(0x2323e98c),
        SPH_C32(0xc3c37f21), SPH_C32(0x18184860), SPH_C32(0x9696cf6e),
        SPH_C32(0x05051b14), SPH_C32(0x9a9aeb5e), SPH_C32(0x0707151c),
        SPH_C32(0x12127e48), SPH_C32(0x8080ad36), SPH_C32(0xe2e298a5),
        SPH_C32(0xebeba781), SPH_C32(0x2727f59c), SPH_C32(0xb2b233fe),
        SPH_C32(0x757550cf), SPH_C32(0x09093f24), SPH_C32(0x8383a43a),
        SPH_C32(0x2c2cc4b0), SPH_C32(0x1a1a4668), SPH_C32(0x1b1b416c),
        SPH_C32(0x6e6e11a3), SPH_C32(0x5a5a9d73), SPH_C32(0xa0a04db6),
        SPH_C32(0x5252a553), SPH_C32(0x3b3ba1ec), SPH_C32(0xd6d61475),
        SPH_C32(0xb3b334fa), SPH_C32(0x2929dfa4), SPH_C32(0xe3e39fa1),
        SPH_C32(0x2f2fcdbc), SPH_C32(0x8484b126), SPH_C32(0x5353a257),
        SPH_C32(0xd1d10169), SPH_C32(0x00000000), SPH_C32(0xededb599),
        SPH_C32(0x2020e080), SPH_C32(0xfcfcc2dd), SPH_C32(0xb1b13af2),
        SPH_C32(0x5b5b9a77), SPH_C32(0x6a6a0db3), SPH_C32(0xcbcb4701),
        SPH_C32(0xbebe17ce), SPH_C32(0x3939afe4), SPH_C32(0x4a4aed33),
        SPH_C32(0x4c4cff2b), SPH_C32(0x5858937b), SPH_C32(0xcfcf5b11),
        SPH_C32(0xd0d0066d), SPH_C32(0xefefbb91), SPH_C32(0xaaaa7b9e),
        SPH_C32(0xfbfbd7c1), SPH_C32(0x4343d217), SPH_C32(0x4d4df82f),
        SPH_C32(0x333399cc), SPH_C32(0x8585b622), SPH_C32(0x4545c00f),
        SPH_C32(0xf9f9d9c9), SPH_C32(0x02020e08), SPH_C32(0x7f7f66e7),
        SPH_C32(0x5050ab5b), SPH_C32(0x3c3cb4f0), SPH_C32(0x9f9ff04a),
        SPH_C32(0xa8a87596), SPH_C32(0x5151ac5f), SPH_C32(0xa3a344ba),
        SPH_C32(0x4040db1b), SPH_C32(0x8f8f800a), SPH_C32(0x9292d37e),
        SPH_C32(0x9d9dfe42), SPH_C32(0x3838a8e0), SPH_C32(0xf5f5fdf9),
        SPH_C32(0xbcbc19c6), SPH_C32(0xb6b62fee), SPH_C32(0xdada3045),
        SPH_C32(0x2121e784), SPH_C32(0x10107040), SPH_C32(0xffffcbd1),
        SPH_C32(0xf3f3efe1), SPH_C32(0xd2d20865), SPH_C32(0xcdcd5519),
        SPH_C32(0x0c0c2430), SPH_C32(0x1313794c), SPH_C32(0xececb29d),
        SPH_C32(0x5f5f8667), SPH_C32(0x9797c86a), SPH_C32(0x4444c70b),
        SPH_C32(0x1717655c), SPH_C32(0xc4c46a3d), SPH_C32(0xa7a758aa),
        SPH_C32(0x7e7e61e3), SPH_C32(0x3d3db3f4), SPH_C32(0x6464278b),
        SPH_C32(0x5d5d886f), SPH_C32(0x19194f64), SPH_C32(0x737342d7),
        SPH_C32(0x60603b9b), SPH_C32(0x8181aa32), SPH_C32(0x4f4ff627),
        SPH_C32(0xdcdc225d), SPH_C32(0x2222ee88), SPH_C32(0x2a2ad6a8),
        SPH_C32(0x9090dd76), SPH_C32(0x88889516), SPH_C32(0x4646c903),
        SPH_C32(0xeeeebc95), SPH_C32(0xb8b805d6), SPH_C32(0x14146c50),
        SPH_C32(0xdede2c55), SPH_C32(0x5e5e8163), SPH_C32(0x0b0b312c),
        SPH_C32(0xdbdb3741), SPH_C32(0xe0e096ad), SPH_C32(0x32329ec8),
        SPH_C32(0x3a3aa6e8), SPH_C32(0x0a0a3628), SPH_C32(0x4949e43f),
        SPH_C32(0x06061218), SPH_C32(0x2424fc90), SPH_C32(0x5c5c8f6b),
        SPH_C32(0xc2c27825), SPH_C32(0xd3d30f61), SPH_C32(0xacac6986),
        SPH_C32(0x62623593), SPH_C32(0x9191da72), SPH_C32(0x9595c662),
        SPH_C32(0xe4e48abd), SPH_C32(0x797974ff), SPH_C32(0xe7e783b1),
        SPH_C32(0xc8c84e0d), SPH_C32(0x373785dc), SPH_C32(0x6d6d18af),
        SPH_C32(0x8d8d8e02), SPH_C32(0xd5d51d79), SPH_C32(0x4e4ef123),
        SPH_C32(0xa9a97292), SPH_C32(0x6c6c1fab), SPH_C32(0x5656b943),
        SPH_C32(0xf4f4fafd), SPH_C32(0xeaeaa085), SPH_C32(0x6565208f),
        SPH_C32(0x7a7a7df3), SPH_C32(0xaeae678e), SPH_C32(0x08083820),
        SPH_C32(0xbaba0bde), SPH_C32(0x787873fb), SPH_C32(0x2525fb94),
        SPH_C32(0x2e2ecab8), SPH_C32(0x1c1c5470), SPH_C32(0xa6a65fae),
        SPH_C32(0xb4b421e6), SPH_C32(0xc6c66435), SPH_C32(0xe8e8ae8d),
        SPH_C32(0xdddd2559), SPH_C32(0x747457cb), SPH_C32(0x1f1f5d7c),
        SPH_C32(0x4b4bea37), SPH_C32(0xbdbd1ec2), SPH_C32(0x8b8b9c1a),
        SPH_C32(0x8a8a9b1e), SPH_C32(0x70704bdb), SPH_C32(0x3e3ebaf8),
        SPH_C32(0xb5b526e2), SPH_C32(0x66662983), SPH_C32(0x4848e33b),
        SPH_C32(0x0303090c), SPH_C32(0xf6f6f4f5), SPH_C32(0x0e0e2a38),
        SPH_C32(0x61613c9f), SPH_C32(0x35358bd4), SPH_C32(0x5757be47),
        SPH_C32(0xb9b902d2), SPH_C32(0x8686bf2e), SPH_C32(0xc1c17129),
        SPH_C32(0x1d1d5374), SPH_C32(0x9e9ef74e), SPH_C32(0xe1e191a9),
        SPH_C32(0xf8f8decd), SPH_C32(0x9898e556), SPH_C32(0x11117744),
        SPH_C32(0x696904bf), SPH_C32(0xd9d93949), SPH_C32(0x8e8e870e),
        SPH_C32(0x9494c166), SPH_C32(0x9b9bec5a), SPH_C32(0x1e1e5a78),
        SPH_C32(0x8787b82a), SPH_C32(0xe9e9a989), SPH_C32(0xcece5c15),
        SPH_C32(0x5555b04f), SPH_C32(0x2828d8a0), SPH_C32(0xdfdf2b51),
        SPH_C32(0x8c8c8906), SPH_C32(0xa1a14ab2), SPH_C32(0x89899212),
        SPH_C32(0x0d0d2334), SPH_C32(0xbfbf10ca), SPH_C32(0xe6e684b5),
        SPH_C32(0x4242d513), SPH_C32(0x686803bb), SPH_C32(0x4141dc1f),
        SPH_C32(0x9999e252), SPH_C32(0x2d2dc3b4), SPH_C32(0x0f0f2d3c),
        SPH_C32(0xb0b03df6), SPH_C32(0x5454b74b), SPH_C32(0xbbbb0cda),
        SPH_C32(0x16166258)
};

static const sph_u32 mixtab1[] = {
        SPH_C32(0x97636332), SPH_C32(0xeb7c7c6f), SPH_C32(0xc777775e),
        SPH_C32(0xf77b7b7a), SPH_C32(0xe5f2f2e8), SPH_C32(0xb76b6b0a),
        SPH_C32(0xa76f6f16), SPH_C32(0x39c5c56d), SPH_C32(0xc0303090),
        SPH_C32(0x04010107), SPH_C32(0x8767672e), SPH_C32(0xac2b2bd1),
        SPH_C32(0xd5fefecc), SPH_C32(0x71d7d713), SPH_C32(0x9aabab7c),
        SPH_C32(0xc3767659), SPH_C32(0x05caca40), SPH_C32(0x3e8282a3),
        SPH_C32(0x09c9c949), SPH_C32(0xef7d7d68), SPH_C32(0xc5fafad0),
        SPH_C32(0x7f595994), SPH_C32(0x074747ce), SPH_C32(0xedf0f0e6),
        SPH_C32(0x82adad6e), SPH_C32(0x7dd4d41a), SPH_C32(0xbea2a243),
        SPH_C32(0x8aafaf60), SPH_C32(0x469c9cf9), SPH_C32(0xa6a4a451),
        SPH_C32(0xd3727245), SPH_C32(0x2dc0c076), SPH_C32(0xeab7b728),
        SPH_C32(0xd9fdfdc5), SPH_C32(0x7a9393d4), SPH_C32(0x982626f2),
        SPH_C32(0xd8363682), SPH_C32(0xfc3f3fbd), SPH_C32(0xf1f7f7f3),
        SPH_C32(0x1dcccc52), SPH_C32(0xd034348c), SPH_C32(0xa2a5a556),
        SPH_C32(0xb9e5e58d), SPH_C32(0xe9f1f1e1), SPH_C32(0xdf71714c),
        SPH_C32(0x4dd8d83e), SPH_C32(0xc4313197), SPH_C32(0x5415156b),
        SPH_C32(0x1004041c), SPH_C32(0x31c7c763), SPH_C32(0x8c2323e9),
        SPH_C32(0x21c3c37f), SPH_C32(0x60181848), SPH_C32(0x6e9696cf),
        SPH_C32(0x1405051b), SPH_C32(0x5e9a9aeb), SPH_C32(0x1c070715),
        SPH_C32(0x4812127e), SPH_C32(0x368080ad), SPH_C32(0xa5e2e298),
        SPH_C32(0x81ebeba7), SPH_C32(0x9c2727f5), SPH_C32(0xfeb2b233),
        SPH_C32(0xcf757550), SPH_C32(0x2409093f), SPH_C32(0x3a8383a4),
        SPH_C32(0xb02c2cc4), SPH_C32(0x681a1a46), SPH_C32(0x6c1b1b41),
        SPH_C32(0xa36e6e11), SPH_C32(0x735a5a9d), SPH_C32(0xb6a0a04d),
        SPH_C32(0x535252a5), SPH_C32(0xec3b3ba1), SPH_C32(0x75d6d614),
        SPH_C32(0xfab3b334), SPH_C32(0xa42929df), SPH_C32(0xa1e3e39f),
        SPH_C32(0xbc2f2fcd), SPH_C32(0x268484b1), SPH_C32(0x575353a2),
        SPH_C32(0x69d1d101), SPH_C32(0x00000000), SPH_C32(0x99ededb5),
        SPH_C32(0x802020e0), SPH_C32(0xddfcfcc2), SPH_C32(0xf2b1b13a),
        SPH_C32(0x775b5b9a), SPH_C32(0xb36a6a0d), SPH_C32(0x01cbcb47),
        SPH_C32(0xcebebe17), SPH_C32(0xe43939af), SPH_C32(0x334a4aed),
        SPH_C32(0x2b4c4cff), SPH_C32(0x7b585893), SPH_C32(0x11cfcf5b),
        SPH_C32(0x6dd0d006), SPH_C32(0x91efefbb), SPH_C32(0x9eaaaa7b),
        SPH_C32(0xc1fbfbd7), SPH_C32(0x174343d2), SPH_C32(0x2f4d4df8),
        SPH_C32(0xcc333399), SPH_C32(0x228585b6), SPH_C32(0x0f4545c0),
        SPH_C32(0xc9f9f9d9), SPH_C32(0x0802020e), SPH_C32(0xe77f7f66),
        SPH_C32(0x5b5050ab), SPH_C32(0xf03c3cb4), SPH_C32(0x4a9f9ff0),
        SPH_C32(0x96a8a875), SPH_C32(0x5f5151ac), SPH_C32(0xbaa3a344),
        SPH_C32(0x1b4040db), SPH_C32(0x0a8f8f80), SPH_C32(0x7e9292d3),
        SPH_C32(0x429d9dfe), SPH_C32(0xe03838a8), SPH_C32(0xf9f5f5fd),
        SPH_C32(0xc6bcbc19), SPH_C32(0xeeb6b62f), SPH_C32(0x45dada30),
        SPH_C32(0x842121e7), SPH_C32(0x40101070), SPH_C32(0xd1ffffcb),
        SPH_C32(0xe1f3f3ef), SPH_C32(0x65d2d208), SPH_C32(0x19cdcd55),
        SPH_C32(0x300c0c24), SPH_C32(0x4c131379), SPH_C32(0x9dececb2),
        SPH_C32(0x675f5f86), SPH_C32(0x6a9797c8), SPH_C32(0x0b4444c7),
        SPH_C32(0x5c171765), SPH_C32(0x3dc4c46a), SPH_C32(0xaaa7a758),
        SPH_C32(0xe37e7e61), SPH_C32(0xf43d3db3), SPH_C32(0x8b646427),
        SPH_C32(0x6f5d5d88), SPH_C32(0x6419194f), SPH_C32(0xd7737342),
        SPH_C32(0x9b60603b), SPH_C32(0x328181aa), SPH_C32(0x274f4ff6),
        SPH_C32(0x5ddcdc22), SPH_C32(0x882222ee), SPH_C32(0xa82a2ad6),
        SPH_C32(0x769090dd), SPH_C32(0x16888895), SPH_C32(0x034646c9),
        SPH_C32(0x95eeeebc), SPH_C32(0xd6b8b805), SPH_C32(0x5014146c),
        SPH_C32(0x55dede2c), SPH_C32(0x635e5e81), SPH_C32(0x2c0b0b31),
        SPH_C32(0x41dbdb37), SPH_C32(0xade0e096), SPH_C32(0xc832329e),
        SPH_C32(0xe83a3aa6), SPH_C32(0x280a0a36), SPH_C32(0x3f4949e4),
        SPH_C32(0x18060612), SPH_C32(0x902424fc), SPH_C32(0x6b5c5c8f),
        SPH_C32(0x25c2c278), SPH_C32(0x61d3d30f), SPH_C32(0x86acac69),
        SPH_C32(0x93626235), SPH_C32(0x729191da), SPH_C32(0x629595c6),
        SPH_C32(0xbde4e48a), SPH_C32(0xff797974), SPH_C32(0xb1e7e783),
        SPH_C32(0x0dc8c84e), SPH_C32(0xdc373785), SPH_C32(0xaf6d6d18),
        SPH_C32(0x028d8d8e), SPH_C32(0x79d5d51d), SPH_C32(0x234e4ef1),
        SPH_C32(0x92a9a972), SPH_C32(0xab6c6c1f), SPH_C32(0x435656b9),
        SPH_C32(0xfdf4f4fa), SPH_C32(0x85eaeaa0), SPH_C32(0x8f656520),
        SPH_C32(0xf37a7a7d), SPH_C32(0x8eaeae67), SPH_C32(0x20080838),
        SPH_C32(0xdebaba0b), SPH_C32(0xfb787873), SPH_C32(0x942525fb),
        SPH_C32(0xb82e2eca), SPH_C32(0x701c1c54), SPH_C32(0xaea6a65f),
        SPH_C32(0xe6b4b421), SPH_C32(0x35c6c664), SPH_C32(0x8de8e8ae),
        SPH_C32(0x59dddd25), SPH_C32(0xcb747457), SPH_C32(0x7c1f1f5d),
        SPH_C32(0x374b4bea), SPH_C32(0xc2bdbd1e), SPH_C32(0x1a8b8b9c),
        SPH_C32(0x1e8a8a9b), SPH_C32(0xdb70704b), SPH_C32(0xf83e3eba),
        SPH_C32(0xe2b5b526), SPH_C32(0x83666629), SPH_C32(0x3b4848e3),
        SPH_C32(0x0c030309), SPH_C32(0xf5f6f6f4), SPH_C32(0x380e0e2a),
        SPH_C32(0x9f61613c), SPH_C32(0xd435358b), SPH_C32(0x475757be),
        SPH_C32(0xd2b9b902), SPH_C32(0x2e8686bf), SPH_C32(0x29c1c171),
        SPH_C32(0x741d1d53), SPH_C32(0x4e9e9ef7), SPH_C32(0xa9e1e191),
        SPH_C32(0xcdf8f8de), SPH_C32(0x569898e5), SPH_C32(0x44111177),
        SPH_C32(0xbf696904), SPH_C32(0x49d9d939), SPH_C32(0x0e8e8e87),
        SPH_C32(0x669494c1), SPH_C32(0x5a9b9bec), SPH_C32(0x781e1e5a),
        SPH_C32(0x2a8787b8), SPH_C32(0x89e9e9a9), SPH_C32(0x15cece5c),
        SPH_C32(0x4f5555b0), SPH_C32(0xa02828d8), SPH_C32(0x51dfdf2b),
        SPH_C32(0x068c8c89), SPH_C32(0xb2a1a14a), SPH_C32(0x12898992),
        SPH_C32(0x340d0d23), SPH_C32(0xcabfbf10), SPH_C32(0xb5e6e684),
        SPH_C32(0x134242d5), SPH_C32(0xbb686803), SPH_C32(0x1f4141dc),
        SPH_C32(0x529999e2), SPH_C32(0xb42d2dc3), SPH_C32(0x3c0f0f2d),
        SPH_C32(0xf6b0b03d), SPH_C32(0x4b5454b7), SPH_C32(0xdabbbb0c),
        SPH_C32(0x58161662)
};

static const sph_u32 mixtab2[] = {
        SPH_C32(0x32976363), SPH_C32(0x6feb7c7c), SPH_C32(0x5ec77777),
        SPH_C32(0x7af77b7b), SPH_C32(0xe8e5f2f2), SPH_C32(0x0ab76b6b),
        SPH_C32(0x16a76f6f), SPH_C32(0x6d39c5c5), SPH_C32(0x90c03030),
        SPH_C32(0x07040101), SPH_C32(0x2e876767), SPH_C32(0xd1ac2b2b),
        SPH_C32(0xccd5fefe), SPH_C32(0x1371d7d7), SPH_C32(0x7c9aabab),
        SPH_C32(0x59c37676), SPH_C32(0x4005caca), SPH_C32(0xa33e8282),
        SPH_C32(0x4909c9c9), SPH_C32(0x68ef7d7d), SPH_C32(0xd0c5fafa),
        SPH_C32(0x947f5959), SPH_C32(0xce074747), SPH_C32(0xe6edf0f0),
        SPH_C32(0x6e82adad), SPH_C32(0x1a7dd4d4), SPH_C32(0x43bea2a2),
        SPH_C32(0x608aafaf), SPH_C32(0xf9469c9c), SPH_C32(0x51a6a4a4),
        SPH_C32(0x45d37272), SPH_C32(0x762dc0c0), SPH_C32(0x28eab7b7),
        SPH_C32(0xc5d9fdfd), SPH_C32(0xd47a9393), SPH_C32(0xf2982626),
        SPH_C32(0x82d83636), SPH_C32(0xbdfc3f3f), SPH_C32(0xf3f1f7f7),
        SPH_C32(0x521dcccc), SPH_C32(0x8cd03434), SPH_C32(0x56a2a5a5),
        SPH_C32(0x8db9e5e5), SPH_C32(0xe1e9f1f1), SPH_C32(0x4cdf7171),
        SPH_C32(0x3e4dd8d8), SPH_C32(0x97c43131), SPH_C32(0x6b541515),
        SPH_C32(0x1c100404), SPH_C32(0x6331c7c7), SPH_C32(0xe98c2323),
        SPH_C32(0x7f21c3c3), SPH_C32(0x48601818), SPH_C32(0xcf6e9696),
        SPH_C32(0x1b140505), SPH_C32(0xeb5e9a9a), SPH_C32(0x151c0707),
        SPH_C32(0x7e481212), SPH_C32(0xad368080), SPH_C32(0x98a5e2e2),
        SPH_C32(0xa781ebeb), SPH_C32(0xf59c2727), SPH_C32(0x33feb2b2),
        SPH_C32(0x50cf7575), SPH_C32(0x3f240909), SPH_C32(0xa43a8383),
        SPH_C32(0xc4b02c2c), SPH_C32(0x46681a1a), SPH_C32(0x416c1b1b),
        SPH_C32(0x11a36e6e), SPH_C32(0x9d735a5a), SPH_C32(0x4db6a0a0),
        SPH_C32(0xa5535252), SPH_C32(0xa1ec3b3b), SPH_C32(0x1475d6d6),
        SPH_C32(0x34fab3b3), SPH_C32(0xdfa42929), SPH_C32(0x9fa1e3e3),
        SPH_C32(0xcdbc2f2f), SPH_C32(0xb1268484), SPH_C32(0xa2575353),
        SPH_C32(0x0169d1d1), SPH_C32(0x00000000), SPH_C32(0xb599eded),
        SPH_C32(0xe0802020), SPH_C32(0xc2ddfcfc), SPH_C32(0x3af2b1b1),
        SPH_C32(0x9a775b5b), SPH_C32(0x0db36a6a), SPH_C32(0x4701cbcb),
        SPH_C32(0x17cebebe), SPH_C32(0xafe43939), SPH_C32(0xed334a4a),
        SPH_C32(0xff2b4c4c), SPH_C32(0x937b5858), SPH_C32(0x5b11cfcf),
        SPH_C32(0x066dd0d0), SPH_C32(0xbb91efef), SPH_C32(0x7b9eaaaa),
        SPH_C32(0xd7c1fbfb), SPH_C32(0xd2174343), SPH_C32(0xf82f4d4d),
        SPH_C32(0x99cc3333), SPH_C32(0xb6228585), SPH_C32(0xc00f4545),
        SPH_C32(0xd9c9f9f9), SPH_C32(0x0e080202), SPH_C32(0x66e77f7f),
        SPH_C32(0xab5b5050), SPH_C32(0xb4f03c3c), SPH_C32(0xf04a9f9f),
        SPH_C32(0x7596a8a8), SPH_C32(0xac5f5151), SPH_C32(0x44baa3a3),
        SPH_C32(0xdb1b4040), SPH_C32(0x800a8f8f), SPH_C32(0xd37e9292),
        SPH_C32(0xfe429d9d), SPH_C32(0xa8e03838), SPH_C32(0xfdf9f5f5),
        SPH_C32(0x19c6bcbc), SPH_C32(0x2feeb6b6), SPH_C32(0x3045dada),
        SPH_C32(0xe7842121), SPH_C32(0x70401010), SPH_C32(0xcbd1ffff),
        SPH_C32(0xefe1f3f3), SPH_C32(0x0865d2d2), SPH_C32(0x5519cdcd),
        SPH_C32(0x24300c0c), SPH_C32(0x794c1313), SPH_C32(0xb29decec),
        SPH_C32(0x86675f5f), SPH_C32(0xc86a9797), SPH_C32(0xc70b4444),
        SPH_C32(0x655c1717), SPH_C32(0x6a3dc4c4), SPH_C32(0x58aaa7a7),
        SPH_C32(0x61e37e7e), SPH_C32(0xb3f43d3d), SPH_C32(0x278b6464),
        SPH_C32(0x886f5d5d), SPH_C32(0x4f641919), SPH_C32(0x42d77373),
        SPH_C32(0x3b9b6060), SPH_C32(0xaa328181), SPH_C32(0xf6274f4f),
        SPH_C32(0x225ddcdc), SPH_C32(0xee882222), SPH_C32(0xd6a82a2a),
        SPH_C32(0xdd769090), SPH_C32(0x95168888), SPH_C32(0xc9034646),
        SPH_C32(0xbc95eeee), SPH_C32(0x05d6b8b8), SPH_C32(0x6c501414),
        SPH_C32(0x2c55dede), SPH_C32(0x81635e5e), SPH_C32(0x312c0b0b),
        SPH_C32(0x3741dbdb), SPH_C32(0x96ade0e0), SPH_C32(0x9ec83232),
        SPH_C32(0xa6e83a3a), SPH_C32(0x36280a0a), SPH_C32(0xe43f4949),
        SPH_C32(0x12180606), SPH_C32(0xfc902424), SPH_C32(0x8f6b5c5c),
        SPH_C32(0x7825c2c2), SPH_C32(0x0f61d3d3), SPH_C32(0x6986acac),
        SPH_C32(0x35936262), SPH_C32(0xda729191), SPH_C32(0xc6629595),
        SPH_C32(0x8abde4e4), SPH_C32(0x74ff7979), SPH_C32(0x83b1e7e7),
        SPH_C32(0x4e0dc8c8), SPH_C32(0x85dc3737), SPH_C32(0x18af6d6d),
        SPH_C32(0x8e028d8d), SPH_C32(0x1d79d5d5), SPH_C32(0xf1234e4e),
        SPH_C32(0x7292a9a9), SPH_C32(0x1fab6c6c), SPH_C32(0xb9435656),
        SPH_C32(0xfafdf4f4), SPH_C32(0xa085eaea), SPH_C32(0x208f6565),
        SPH_C32(0x7df37a7a), SPH_C32(0x678eaeae), SPH_C32(0x38200808),
        SPH_C32(0x0bdebaba), SPH_C32(0x73fb7878), SPH_C32(0xfb942525),
        SPH_C32(0xcab82e2e), SPH_C32(0x54701c1c), SPH_C32(0x5faea6a6),
        SPH_C32(0x21e6b4b4), SPH_C32(0x6435c6c6), SPH_C32(0xae8de8e8),
        SPH_C32(0x2559dddd), SPH_C32(0x57cb7474), SPH_C32(0x5d7c1f1f),
        SPH_C32(0xea374b4b), SPH_C32(0x1ec2bdbd), SPH_C32(0x9c1a8b8b),
        SPH_C32(0x9b1e8a8a), SPH_C32(0x4bdb7070), SPH_C32(0xbaf83e3e),
        SPH_C32(0x26e2b5b5), SPH_C32(0x29836666), SPH_C32(0xe33b4848),
        SPH_C32(0x090c0303), SPH_C32(0xf4f5f6f6), SPH_C32(0x2a380e0e),
        SPH_C32(0x3c9f6161), SPH_C32(0x8bd43535), SPH_C32(0xbe475757),
        SPH_C32(0x02d2b9b9), SPH_C32(0xbf2e8686), SPH_C32(0x7129c1c1),
        SPH_C32(0x53741d1d), SPH_C32(0xf74e9e9e), SPH_C32(0x91a9e1e1),
        SPH_C32(0xdecdf8f8), SPH_C32(0xe5569898), SPH_C32(0x77441111),
        SPH_C32(0x04bf6969), SPH_C32(0x3949d9d9), SPH_C32(0x870e8e8e),
        SPH_C32(0xc1669494), SPH_C32(0xec5a9b9b), SPH_C32(0x5a781e1e),
        SPH_C32(0xb82a8787), SPH_C32(0xa989e9e9), SPH_C32(0x5c15cece),
        SPH_C32(0xb04f5555), SPH_C32(0xd8a02828), SPH_C32(0x2b51dfdf),
        SPH_C32(0x89068c8c), SPH_C32(0x4ab2a1a1), SPH_C32(0x92128989),
        SPH_C32(0x23340d0d), SPH_C32(0x10cabfbf), SPH_C32(0x84b5e6e6),
        SPH_C32(0xd5134242), SPH_C32(0x03bb6868), SPH_C32(0xdc1f4141),
        SPH_C32(0xe2529999), SPH_C32(0xc3b42d2d), SPH_C32(0x2d3c0f0f),
        SPH_C32(0x3df6b0b0), SPH_C32(0xb74b5454), SPH_C32(0x0cdabbbb),
        SPH_C32(0x62581616)
};

static const sph_u32 mixtab3[] = {
        SPH_C32(0x63329763), SPH_C32(0x7c6feb7c), SPH_C32(0x775ec777),
        SPH_C32(0x7b7af77b), SPH_C32(0xf2e8e5f2), SPH_C32(0x6b0ab76b),
        SPH_C32(0x6f16a76f), SPH_C32(0xc56d39c5), SPH_C32(0x3090c030),
        SPH_C32(0x01070401), SPH_C32(0x672e8767), SPH_C32(0x2bd1ac2b),
        SPH_C32(0xfeccd5fe), SPH_C32(0xd71371d7), SPH_C32(0xab7c9aab),
        SPH_C32(0x7659c376), SPH_C32(0xca4005ca), SPH_C32(0x82a33e82),
        SPH_C32(0xc94909c9), SPH_C32(0x7d68ef7d), SPH_C32(0xfad0c5fa),
        SPH_C32(0x59947f59), SPH_C32(0x47ce0747), SPH_C32(0xf0e6edf0),
        SPH_C32(0xad6e82ad), SPH_C32(0xd41a7dd4), SPH_C32(0xa243bea2),
        SPH_C32(0xaf608aaf), SPH_C32(0x9cf9469c), SPH_C32(0xa451a6a4),
        SPH_C32(0x7245d372), SPH_C32(0xc0762dc0), SPH_C32(0xb728eab7),
        SPH_C32(0xfdc5d9fd), SPH_C32(0x93d47a93), SPH_C32(0x26f29826),
        SPH_C32(0x3682d836), SPH_C32(0x3fbdfc3f), SPH_C32(0xf7f3f1f7),
        SPH_C32(0xcc521dcc), SPH_C32(0x348cd034), SPH_C32(0xa556a2a5),
        SPH_C32(0xe58db9e5), SPH_C32(0xf1e1e9f1), SPH_C32(0x714cdf71),
        SPH_C32(0xd83e4dd8), SPH_C32(0x3197c431), SPH_C32(0x156b5415),
        SPH_C32(0x041c1004), SPH_C32(0xc76331c7), SPH_C32(0x23e98c23),
        SPH_C32(0xc37f21c3), SPH_C32(0x18486018), SPH_C32(0x96cf6e96),
        SPH_C32(0x051b1405), SPH_C32(0x9aeb5e9a), SPH_C32(0x07151c07),
        SPH_C32(0x127e4812), SPH_C32(0x80ad3680), SPH_C32(0xe298a5e2),
        SPH_C32(0xeba781eb), SPH_C32(0x27f59c27), SPH_C32(0xb233feb2),
        SPH_C32(0x7550cf75), SPH_C32(0x093f2409), SPH_C32(0x83a43a83),
        SPH_C32(0x2cc4b02c), SPH_C32(0x1a46681a), SPH_C32(0x1b416c1b),
        SPH_C32(0x6e11a36e), SPH_C32(0x5a9d735a), SPH_C32(0xa04db6a0),
        SPH_C32(0x52a55352), SPH_C32(0x3ba1ec3b), SPH_C32(0xd61475d6),
        SPH_C32(0xb334fab3), SPH_C32(0x29dfa429), SPH_C32(0xe39fa1e3),
        SPH_C32(0x2fcdbc2f), SPH_C32(0x84b12684), SPH_C32(0x53a25753),
        SPH_C32(0xd10169d1), SPH_C32(0x00000000), SPH_C32(0xedb599ed),
        SPH_C32(0x20e08020), SPH_C32(0xfcc2ddfc), SPH_C32(0xb13af2b1),
        SPH_C32(0x5b9a775b), SPH_C32(0x6a0db36a), SPH_C32(0xcb4701cb),
        SPH_C32(0xbe17cebe), SPH_C32(0x39afe439), SPH_C32(0x4aed334a),
        SPH_C32(0x4cff2b4c), SPH_C32(0x58937b58), SPH_C32(0xcf5b11cf),
        SPH_C32(0xd0066dd0), SPH_C32(0xefbb91ef), SPH_C32(0xaa7b9eaa),
        SPH_C32(0xfbd7c1fb), SPH_C32(0x43d21743), SPH_C32(0x4df82f4d),
        SPH_C32(0x3399cc33), SPH_C32(0x85b62285), SPH_C32(0x45c00f45),
        SPH_C32(0xf9d9c9f9), SPH_C32(0x020e0802), SPH_C32(0x7f66e77f),
        SPH_C32(0x50ab5b50), SPH_C32(0x3cb4f03c), SPH_C32(0x9ff04a9f),
        SPH_C32(0xa87596a8), SPH_C32(0x51ac5f51), SPH_C32(0xa344baa3),
        SPH_C32(0x40db1b40), SPH_C32(0x8f800a8f), SPH_C32(0x92d37e92),
        SPH_C32(0x9dfe429d), SPH_C32(0x38a8e038), SPH_C32(0xf5fdf9f5),
        SPH_C32(0xbc19c6bc), SPH_C32(0xb62feeb6), SPH_C32(0xda3045da),
        SPH_C32(0x21e78421), SPH_C32(0x10704010), SPH_C32(0xffcbd1ff),
        SPH_C32(0xf3efe1f3), SPH_C32(0xd20865d2), SPH_C32(0xcd5519cd),
        SPH_C32(0x0c24300c), SPH_C32(0x13794c13), SPH_C32(0xecb29dec),
        SPH_C32(0x5f86675f), SPH_C32(0x97c86a97), SPH_C32(0x44c70b44),
        SPH_C32(0x17655c17), SPH_C32(0xc46a3dc4), SPH_C32(0xa758aaa7),
        SPH_C32(0x7e61e37e), SPH_C32(0x3db3f43d), SPH_C32(0x64278b64),
        SPH_C32(0x5d886f5d), SPH_C32(0x194f6419), SPH_C32(0x7342d773),
        SPH_C32(0x603b9b60), SPH_C32(0x81aa3281), SPH_C32(0x4ff6274f),
        SPH_C32(0xdc225ddc), SPH_C32(0x22ee8822), SPH_C32(0x2ad6a82a),
        SPH_C32(0x90dd7690), SPH_C32(0x88951688), SPH_C32(0x46c90346),
        SPH_C32(0xeebc95ee), SPH_C32(0xb805d6b8), SPH_C32(0x146c5014),
        SPH_C32(0xde2c55de), SPH_C32(0x5e81635e), SPH_C32(0x0b312c0b),
        SPH_C32(0xdb3741db), SPH_C32(0xe096ade0), SPH_C32(0x329ec832),
        SPH_C32(0x3aa6e83a), SPH_C32(0x0a36280a), SPH_C32(0x49e43f49),
        SPH_C32(0x06121806), SPH_C32(0x24fc9024), SPH_C32(0x5c8f6b5c),
        SPH_C32(0xc27825c2), SPH_C32(0xd30f61d3), SPH_C32(0xac6986ac),
        SPH_C32(0x62359362), SPH_C32(0x91da7291), SPH_C32(0x95c66295),
        SPH_C32(0xe48abde4), SPH_C32(0x7974ff79), SPH_C32(0xe783b1e7),
        SPH_C32(0xc84e0dc8), SPH_C32(0x3785dc37), SPH_C32(0x6d18af6d),
        SPH_C32(0x8d8e028d), SPH_C32(0xd51d79d5), SPH_C32(0x4ef1234e),
        SPH_C32(0xa97292a9), SPH_C32(0x6c1fab6c), SPH_C32(0x56b94356),
        SPH_C32(0xf4fafdf4), SPH_C32(0xeaa085ea), SPH_C32(0x65208f65),
        SPH_C32(0x7a7df37a), SPH_C32(0xae678eae), SPH_C32(0x08382008),
        SPH_C32(0xba0bdeba), SPH_C32(0x7873fb78), SPH_C32(0x25fb9425),
        SPH_C32(0x2ecab82e), SPH_C32(0x1c54701c), SPH_C32(0xa65faea6),
        SPH_C32(0xb421e6b4), SPH_C32(0xc66435c6), SPH_C32(0xe8ae8de8),
        SPH_C32(0xdd2559dd), SPH_C32(0x7457cb74), SPH_C32(0x1f5d7c1f),
        SPH_C32(0x4bea374b), SPH_C32(0xbd1ec2bd), SPH_C32(0x8b9c1a8b),
        SPH_C32(0x8a9b1e8a), SPH_C32(0x704bdb70), SPH_C32(0x3ebaf83e),
        SPH_C32(0xb526e2b5), SPH_C32(0x66298366), SPH_C32(0x48e33b48),
        SPH_C32(0x03090c03), SPH_C32(0xf6f4f5f6), SPH_C32(0x0e2a380e),
        SPH_C32(0x613c9f61), SPH_C32(0x358bd435), SPH_C32(0x57be4757),
        SPH_C32(0xb902d2b9), SPH_C32(0x86bf2e86), SPH_C32(0xc17129c1),
        SPH_C32(0x1d53741d), SPH_C32(0x9ef74e9e), SPH_C32(0xe191a9e1),
        SPH_C32(0xf8decdf8), SPH_C32(0x98e55698), SPH_C32(0x11774411),
        SPH_C32(0x6904bf69), SPH_C32(0xd93949d9), SPH_C32(0x8e870e8e),
        SPH_C32(0x94c16694), SPH_C32(0x9bec5a9b), SPH_C32(0x1e5a781e),
        SPH_C32(0x87b82a87), SPH_C32(0xe9a989e9), SPH_C32(0xce5c15ce),
        SPH_C32(0x55b04f55), SPH_C32(0x28d8a028), SPH_C32(0xdf2b51df),
        SPH_C32(0x8c89068c), SPH_C32(0xa14ab2a1), SPH_C32(0x89921289),
        SPH_C32(0x0d23340d), SPH_C32(0xbf10cabf), SPH_C32(0xe684b5e6),
        SPH_C32(0x42d51342), SPH_C32(0x6803bb68), SPH_C32(0x41dc1f41),
        SPH_C32(0x99e25299), SPH_C32(0x2dc3b42d), SPH_C32(0x0f2d3c0f),
        SPH_C32(0xb03df6b0), SPH_C32(0x54b74b54), SPH_C32(0xbb0cdabb),
        SPH_C32(0x16625816)
};

#define TIX2(q, x00, x01, x08, x10, x24)   do { \
                x10 ^= x00; \
                x00 = (q); \
                x08 ^= x00; \
                x01 ^= x24; \
        } while (0)

#define TIX3(q, x00, x01, x04, x08, x16, x27, x30)   do { \
                x16 ^= x00; \
                x00 = (q); \
                x08 ^= x00; \
                x01 ^= x27; \
                x04 ^= x30; \
        } while (0)

#define TIX4(q, x00, x01, x04, x07, x08, x22, x24, x27, x30)   do { \
                x22 ^= x00; \
                x00 = (q); \
                x08 ^= x00; \
                x01 ^= x24; \
                x04 ^= x27; \
                x07 ^= x30; \
        } while (0)

#define CMIX30(x00, x01, x02, x04, x05, x06, x15, x16, x17)   do { \
                x00 ^= x04; \
                x01 ^= x05; \
                x02 ^= x06; \
                x15 ^= x04; \
                x16 ^= x05; \
                x17 ^= x06; \
        } while (0)

#define CMIX36(x00, x01, x02, x04, x05, x06, x18, x19, x20)   do { \
                x00 ^= x04; \
                x01 ^= x05; \
                x02 ^= x06; \
                x18 ^= x04; \
                x19 ^= x05; \
                x20 ^= x06; \
        } while (0)

#define SMIX(x0, x1, x2, x3)   do { \
                sph_u32 c0 = 0; \
                sph_u32 c1 = 0; \
                sph_u32 c2 = 0; \
                sph_u32 c3 = 0; \
                sph_u32 r0 = 0; \
                sph_u32 r1 = 0; \
                sph_u32 r2 = 0; \
                sph_u32 r3 = 0; \
                sph_u32 tmp; \
                tmp = mixtab0[x0 >> 24]; \
                c0 ^= tmp; \
                tmp = mixtab1[(x0 >> 16) & 0xFF]; \
                c0 ^= tmp; \
                r1 ^= tmp; \
                tmp = mixtab2[(x0 >>  8) & 0xFF]; \
                c0 ^= tmp; \
                r2 ^= tmp; \
                tmp = mixtab3[x0 & 0xFF]; \
                c0 ^= tmp; \
                r3 ^= tmp; \
                tmp = mixtab0[x1 >> 24]; \
                c1 ^= tmp; \
                r0 ^= tmp; \
                tmp = mixtab1[(x1 >> 16) & 0xFF]; \
                c1 ^= tmp; \
                tmp = mixtab2[(x1 >>  8) & 0xFF]; \
                c1 ^= tmp; \
                r2 ^= tmp; \
                tmp = mixtab3[x1 & 0xFF]; \
                c1 ^= tmp; \
                r3 ^= tmp; \
                tmp = mixtab0[x2 >> 24]; \
                c2 ^= tmp; \
                r0 ^= tmp; \
                tmp = mixtab1[(x2 >> 16) & 0xFF]; \
                c2 ^= tmp; \
                r1 ^= tmp; \
                tmp = mixtab2[(x2 >>  8) & 0xFF]; \
                c2 ^= tmp; \
                tmp = mixtab3[x2 & 0xFF]; \
                c2 ^= tmp; \
                r3 ^= tmp; \
                tmp = mixtab0[x3 >> 24]; \
                c3 ^= tmp; \
                r0 ^= tmp; \
                tmp = mixtab1[(x3 >> 16) & 0xFF]; \
                c3 ^= tmp; \
                r1 ^= tmp; \
                tmp = mixtab2[(x3 >>  8) & 0xFF]; \
                c3 ^= tmp; \
                r2 ^= tmp; \
                tmp = mixtab3[x3 & 0xFF]; \
                c3 ^= tmp; \
                x0 = ((c0 ^ r0) & SPH_C32(0xFF000000)) \
                        | ((c1 ^ r1) & SPH_C32(0x00FF0000)) \
                        | ((c2 ^ r2) & SPH_C32(0x0000FF00)) \
                        | ((c3 ^ r3) & SPH_C32(0x000000FF)); \
                x1 = ((c1 ^ (r0 << 8)) & SPH_C32(0xFF000000)) \
                        | ((c2 ^ (r1 << 8)) & SPH_C32(0x00FF0000)) \
                        | ((c3 ^ (r2 << 8)) & SPH_C32(0x0000FF00)) \
                        | ((c0 ^ (r3 >> 24)) & SPH_C32(0x000000FF)); \
                x2 = ((c2 ^ (r0 << 16)) & SPH_C32(0xFF000000)) \
                        | ((c3 ^ (r1 << 16)) & SPH_C32(0x00FF0000)) \
                        | ((c0 ^ (r2 >> 16)) & SPH_C32(0x0000FF00)) \
                        | ((c1 ^ (r3 >> 16)) & SPH_C32(0x000000FF)); \
                x3 = ((c3 ^ (r0 << 24)) & SPH_C32(0xFF000000)) \
                        | ((c0 ^ (r1 >> 8)) & SPH_C32(0x00FF0000)) \
                        | ((c1 ^ (r2 >> 8)) & SPH_C32(0x0000FF00)) \
                        | ((c2 ^ (r3 >> 8)) & SPH_C32(0x000000FF)); \
                /* */ \
        } while (0)


#define fugue_DECL_STATE_SMALL \
        sph_u32 S00, S01, S02, S03, S04, S05, S06, S07, S08, S09; \
        sph_u32 S10, S11, S12, S13, S14, S15, S16, S17, S18, S19; \
        sph_u32 S20, S21, S22, S23, S24, S25, S26, S27, S28, S29;

#define fugue_DECL_STATE_BIG \
        fugue_DECL_STATE_SMALL \
        sph_u32 S30, S31, S32, S33, S34, S35;

#define fugue_READ_STATE_SMALL(state)   do { \
                S00 = (state)->S[ 0]; \
                S01 = (state)->S[ 1]; \
                S02 = (state)->S[ 2]; \
                S03 = (state)->S[ 3]; \
                S04 = (state)->S[ 4]; \
                S05 = (state)->S[ 5]; \
                S06 = (state)->S[ 6]; \
                S07 = (state)->S[ 7]; \
                S08 = (state)->S[ 8]; \
                S09 = (state)->S[ 9]; \
                S10 = (state)->S[10]; \
                S11 = (state)->S[11]; \
                S12 = (state)->S[12]; \
                S13 = (state)->S[13]; \
                S14 = (state)->S[14]; \
                S15 = (state)->S[15]; \
                S16 = (state)->S[16]; \
                S17 = (state)->S[17]; \
                S18 = (state)->S[18]; \
                S19 = (state)->S[19]; \
                S20 = (state)->S[20]; \
                S21 = (state)->S[21]; \
                S22 = (state)->S[22]; \
                S23 = (state)->S[23]; \
                S24 = (state)->S[24]; \
                S25 = (state)->S[25]; \
                S26 = (state)->S[26]; \
                S27 = (state)->S[27]; \
                S28 = (state)->S[28]; \
                S29 = (state)->S[29]; \
        } while (0)

#define fugue_READ_STATE_BIG(state)   do { \
                fugue_READ_STATE_SMALL(state); \
                S30 = (state)->S[30]; \
                S31 = (state)->S[31]; \
                S32 = (state)->S[32]; \
                S33 = (state)->S[33]; \
                S34 = (state)->S[34]; \
                S35 = (state)->S[35]; \
        } while (0)

#define fugue_WRITE_STATE_SMALL(state)   do { \
                (state)->S[ 0] = S00; \
                (state)->S[ 1] = S01; \
                (state)->S[ 2] = S02; \
                (state)->S[ 3] = S03; \
                (state)->S[ 4] = S04; \
                (state)->S[ 5] = S05; \
                (state)->S[ 6] = S06; \
                (state)->S[ 7] = S07; \
                (state)->S[ 8] = S08; \
                (state)->S[ 9] = S09; \
                (state)->S[10] = S10; \
                (state)->S[11] = S11; \
                (state)->S[12] = S12; \
                (state)->S[13] = S13; \
                (state)->S[14] = S14; \
                (state)->S[15] = S15; \
                (state)->S[16] = S16; \
                (state)->S[17] = S17; \
                (state)->S[18] = S18; \
                (state)->S[19] = S19; \
                (state)->S[20] = S20; \
                (state)->S[21] = S21; \
                (state)->S[22] = S22; \
                (state)->S[23] = S23; \
                (state)->S[24] = S24; \
                (state)->S[25] = S25; \
                (state)->S[26] = S26; \
                (state)->S[27] = S27; \
                (state)->S[28] = S28; \
                (state)->S[29] = S29; \
        } while (0)

#define fugue_WRITE_STATE_BIG(state)   do { \
                fugue_WRITE_STATE_SMALL(state); \
                (state)->S[30] = S30; \
                (state)->S[31] = S31; \
                (state)->S[32] = S32; \
                (state)->S[33] = S33; \
                (state)->S[34] = S34; \
                (state)->S[35] = S35; \
        } while (0)


static void
fugue_init(sph_fugue_context *sc, size_t z_len,
        const sph_u32 *iv, size_t iv_len)
{
        size_t u;

        for (u = 0; u < z_len; u ++)
                sc->S[u] = 0;
        memcpy(&sc->S[z_len], iv, iv_len * sizeof *iv);
        sc->partial = 0;
        sc->partial_len = 0;
        sc->round_shift = 0;
        sc->bit_count = 0;
}


#define INCR_COUNTER   do { \
                sc->bit_count += (sph_u64)len << 3; \
        } while (0)


#define CORE_ENTRY \
        sph_u32 p; \
        unsigned plen, rshift; \
        INCR_COUNTER; \
        p = sc->partial; \
        plen = sc->partial_len; \
        if (plen < 4) { \
                unsigned count = 4 - plen; \
                if (len < count) \
                        count = len; \
                plen += count; \
                while (count -- > 0) { \
                        p = (p << 8) | *(const unsigned char *)data; \
                        data = (const unsigned char *)data + 1; \
                        len --; \
                } \
                if (len == 0) { \
                        sc->partial = p; \
                        sc->partial_len = plen; \
                        return; \
                } \
        }

#define CORE_EXIT \
        p = 0; \
        sc->partial_len = (unsigned)len; \
        while (len -- > 0) { \
                p = (p << 8) | *(const unsigned char *)data; \
                data = (const unsigned char *)data + 1; \
        } \
        sc->partial = p; \
        sc->round_shift = rshift;

#define NEXT(rc) \
        if (len <= 4) { \
                rshift = (rc); \
                break; \
        } \
        p = sph_dec32be(data); \
        data = (const unsigned char *)data + 4; \
        len -= 4

static void
fugue2_core(sph_fugue_context *sc, const void *data, size_t len)
{
        fugue_DECL_STATE_SMALL
        CORE_ENTRY
        fugue_READ_STATE_SMALL(sc);
        rshift = sc->round_shift;
        switch (rshift) {
                for (;;) {
                        sph_u32 q;

                case 0:
                        q = p;
                        TIX2(q, S00, S01, S08, S10, S24);
                        CMIX30(S27, S28, S29, S01, S02, S03, S12, S13, S14);
                        SMIX(S27, S28, S29, S00);
                        CMIX30(S24, S25, S26, S28, S29, S00, S09, S10, S11);
                        SMIX(S24, S25, S26, S27);
                        NEXT(1);
                        /* fall through */
                case 1:
                        q = p;
                        TIX2(q, S24, S25, S02, S04, S18);
                        CMIX30(S21, S22, S23, S25, S26, S27, S06, S07, S08);
                        SMIX(S21, S22, S23, S24);
                        CMIX30(S18, S19, S20, S22, S23, S24, S03, S04, S05);
                        SMIX(S18, S19, S20, S21);
                        NEXT(2);
                        /* fall through */
                case 2:
                        q = p;
                        TIX2(q, S18, S19, S26, S28, S12);
                        CMIX30(S15, S16, S17, S19, S20, S21, S00, S01, S02);
                        SMIX(S15, S16, S17, S18);
                        CMIX30(S12, S13, S14, S16, S17, S18, S27, S28, S29);
                        SMIX(S12, S13, S14, S15);
                        NEXT(3);
                        /* fall through */
                case 3:
                        q = p;
                        TIX2(q, S12, S13, S20, S22, S06);
                        CMIX30(S09, S10, S11, S13, S14, S15, S24, S25, S26);
                        SMIX(S09, S10, S11, S12);
                        CMIX30(S06, S07, S08, S10, S11, S12, S21, S22, S23);
                        SMIX(S06, S07, S08, S09);
                        NEXT(4);
                        /* fall through */
                case 4:
                        q = p;
                        TIX2(q, S06, S07, S14, S16, S00);
                        CMIX30(S03, S04, S05, S07, S08, S09, S18, S19, S20);
                        SMIX(S03, S04, S05, S06);
                        CMIX30(S00, S01, S02, S04, S05, S06, S15, S16, S17);
                        SMIX(S00, S01, S02, S03);
                        NEXT(0);
                }
        }
        CORE_EXIT
        fugue_WRITE_STATE_SMALL(sc);
}

static void
fugue3_core(sph_fugue_context *sc, const void *data, size_t len)
{
        fugue_DECL_STATE_BIG
        CORE_ENTRY
        fugue_READ_STATE_BIG(sc);
        rshift = sc->round_shift;
        switch (rshift) {
                for (;;) {
                        sph_u32 q;

                case 0:
                        q = p;
                        TIX3(q, S00, S01, S04, S08, S16, S27, S30);
                        CMIX36(S33, S34, S35, S01, S02, S03, S15, S16, S17);
                        SMIX(S33, S34, S35, S00);
                        CMIX36(S30, S31, S32, S34, S35, S00, S12, S13, S14);
                        SMIX(S30, S31, S32, S33);
                        CMIX36(S27, S28, S29, S31, S32, S33, S09, S10, S11);
                        SMIX(S27, S28, S29, S30);
                        NEXT(1);
                        /* fall through */
                case 1:
                        q = p;
                        TIX3(q, S27, S28, S31, S35, S07, S18, S21);
                        CMIX36(S24, S25, S26, S28, S29, S30, S06, S07, S08);
                        SMIX(S24, S25, S26, S27);
                        CMIX36(S21, S22, S23, S25, S26, S27, S03, S04, S05);
                        SMIX(S21, S22, S23, S24);
                        CMIX36(S18, S19, S20, S22, S23, S24, S00, S01, S02);
                        SMIX(S18, S19, S20, S21);
                        NEXT(2);
                        /* fall through */
                case 2:
                        q = p;
                        TIX3(q, S18, S19, S22, S26, S34, S09, S12);
                        CMIX36(S15, S16, S17, S19, S20, S21, S33, S34, S35);
                        SMIX(S15, S16, S17, S18);
                        CMIX36(S12, S13, S14, S16, S17, S18, S30, S31, S32);
                        SMIX(S12, S13, S14, S15);
                        CMIX36(S09, S10, S11, S13, S14, S15, S27, S28, S29);
                        SMIX(S09, S10, S11, S12);
                        NEXT(3);
                        /* fall through */
                case 3:
                        q = p;
                        TIX3(q, S09, S10, S13, S17, S25, S00, S03);
                        CMIX36(S06, S07, S08, S10, S11, S12, S24, S25, S26);
                        SMIX(S06, S07, S08, S09);
                        CMIX36(S03, S04, S05, S07, S08, S09, S21, S22, S23);
                        SMIX(S03, S04, S05, S06);
                        CMIX36(S00, S01, S02, S04, S05, S06, S18, S19, S20);
                        SMIX(S00, S01, S02, S03);
                        NEXT(0);
                }
        }
        CORE_EXIT
        fugue_WRITE_STATE_BIG(sc);
}

static void
fugue4_core(sph_fugue_context *sc, const void *data, size_t len)
{
        fugue_DECL_STATE_BIG
        CORE_ENTRY
        fugue_READ_STATE_BIG(sc);
        rshift = sc->round_shift;
        switch (rshift) {
                for (;;) {
                        sph_u32 q;

                case 0:
                        q = p;
                        TIX4(q, S00, S01, S04, S07, S08, S22, S24, S27, S30);
                        CMIX36(S33, S34, S35, S01, S02, S03, S15, S16, S17);
                        SMIX(S33, S34, S35, S00);
                        CMIX36(S30, S31, S32, S34, S35, S00, S12, S13, S14);
                        SMIX(S30, S31, S32, S33);
                        CMIX36(S27, S28, S29, S31, S32, S33, S09, S10, S11);
                        SMIX(S27, S28, S29, S30);
                        CMIX36(S24, S25, S26, S28, S29, S30, S06, S07, S08);
                        SMIX(S24, S25, S26, S27);
                        NEXT(1);
                        /* fall through */
                case 1:
                        q = p;
                        TIX4(q, S24, S25, S28, S31, S32, S10, S12, S15, S18);
                        CMIX36(S21, S22, S23, S25, S26, S27, S03, S04, S05);
                        SMIX(S21, S22, S23, S24);
                        CMIX36(S18, S19, S20, S22, S23, S24, S00, S01, S02);
                        SMIX(S18, S19, S20, S21);
                        CMIX36(S15, S16, S17, S19, S20, S21, S33, S34, S35);
                        SMIX(S15, S16, S17, S18);
                        CMIX36(S12, S13, S14, S16, S17, S18, S30, S31, S32);
                        SMIX(S12, S13, S14, S15);
                        NEXT(2);
                        /* fall through */
                case 2:
                        q = p;
                        TIX4(q, S12, S13, S16, S19, S20, S34, S00, S03, S06);
                        CMIX36(S09, S10, S11, S13, S14, S15, S27, S28, S29);
                        SMIX(S09, S10, S11, S12);
                        CMIX36(S06, S07, S08, S10, S11, S12, S24, S25, S26);
                        SMIX(S06, S07, S08, S09);
                        CMIX36(S03, S04, S05, S07, S08, S09, S21, S22, S23);
                        SMIX(S03, S04, S05, S06);
                        CMIX36(S00, S01, S02, S04, S05, S06, S18, S19, S20);
                        SMIX(S00, S01, S02, S03);
                        NEXT(0);
                }
        }
        CORE_EXIT
        fugue_WRITE_STATE_BIG(sc);
}


#define WRITE_COUNTER   do { \
                sph_enc64be(buf + 4, sc->bit_count + n); \
        } while (0)


#define CLOSE_ENTRY(s, rcm, core) \
        unsigned char buf[16]; \
        unsigned plen, rms; \
        unsigned char *out; \
        sph_u32 S[s]; \
        plen = sc->partial_len; \
        WRITE_COUNTER; \
        if (plen == 0 && n == 0) { \
                plen = 4; \
        } else if (plen < 4 || n != 0) { \
                unsigned u; \
 \
                if (plen == 4) \
                        plen = 0; \
                buf[plen] = ub & ~(0xFFU >> n); \
                for (u = plen + 1; u < 4; u ++) \
                        buf[u] = 0; \
        } \
        core(sc, buf + plen, (sizeof buf) - plen); \
        rms = sc->round_shift * (rcm); \
        memcpy(S, sc->S + (s) - rms, rms * sizeof(sph_u32)); \
        memcpy(S + rms, sc->S, ((s) - rms) * sizeof(sph_u32));

#define ROR(n, s)   do { \
                sph_u32 tmp[n]; \
                memcpy(tmp, S + ((s) - (n)), (n) * sizeof(sph_u32)); \
                memmove(S + (n), S, ((s) - (n)) * sizeof(sph_u32)); \
                memcpy(S, tmp, (n) * sizeof(sph_u32)); \
        } while (0)

static void
fugue2_close(sph_fugue_context *sc, unsigned ub, unsigned n,
    unsigned char *dst, size_t out_size_w32)
{
        int i;

        CLOSE_ENTRY(30, 6, fugue2_core)
        for (i = 0; i < 10; i ++) {
                ROR(3, 30);
                CMIX30(S[0], S[1], S[2], S[4], S[5], S[6], S[15], S[16], S[17]);
                SMIX(S[0], S[1], S[2], S[3]);
        }
        for (i = 0; i < 13; i ++) {
                S[4] ^= S[0];
                S[15] ^= S[0];
                ROR(15, 30);
                SMIX(S[0], S[1], S[2], S[3]);
                S[4] ^= S[0];
                S[16] ^= S[0];
                ROR(14, 30);
                SMIX(S[0], S[1], S[2], S[3]);
        }
        S[4] ^= S[0];
        S[15] ^= S[0];
    out = dst;  // unsigned char *out
        sph_enc32be(out +  0, S[ 1]);
        sph_enc32be(out +  4, S[ 2]);
        sph_enc32be(out +  8, S[ 3]);
        sph_enc32be(out + 12, S[ 4]);
        sph_enc32be(out + 16, S[15]);
        sph_enc32be(out + 20, S[16]);
        sph_enc32be(out + 24, S[17]);
        if (out_size_w32 == 8) {
                sph_enc32be(out + 28, S[18]);
                sph_fugue256_init(sc);
        } else {
                sph_fugue224_init(sc);
        }
}

static void
fugue3_close(sph_fugue_context *sc, unsigned ub, unsigned n, unsigned char *dst)
{
        int i;

        CLOSE_ENTRY(36, 9, fugue3_core)
        for (i = 0; i < 18; i ++) {
                ROR(3, 36);
                CMIX36(S[0], S[1], S[2], S[4], S[5], S[6], S[18], S[19], S[20]);
                SMIX(S[0], S[1], S[2], S[3]);
        }
        for (i = 0; i < 13; i ++) {
                S[4] ^= S[0];
                S[12] ^= S[0];
                S[24] ^= S[0];
                ROR(12, 36);
                SMIX(S[0], S[1], S[2], S[3]);
                S[4] ^= S[0];
                S[13] ^= S[0];
                S[24] ^= S[0];
                ROR(12, 36);
                SMIX(S[0], S[1], S[2], S[3]);
                S[4] ^= S[0];
                S[13] ^= S[0];
                S[25] ^= S[0];
                ROR(11, 36);
                SMIX(S[0], S[1], S[2], S[3]);
        }
        S[4] ^= S[0];
        S[12] ^= S[0];
        S[24] ^= S[0];
    out = dst; // unsigned char *out;
        sph_enc32be(out +  0, S[ 1]);
        sph_enc32be(out +  4, S[ 2]);
        sph_enc32be(out +  8, S[ 3]);
        sph_enc32be(out + 12, S[ 4]);
        sph_enc32be(out + 16, S[12]);
        sph_enc32be(out + 20, S[13]);
        sph_enc32be(out + 24, S[14]);
        sph_enc32be(out + 28, S[15]);
        sph_enc32be(out + 32, S[24]);
        sph_enc32be(out + 36, S[25]);
        sph_enc32be(out + 40, S[26]);
        sph_enc32be(out + 44, S[27]);
        sph_fugue384_init(sc);
}

static void
fugue4_close(sph_fugue_context *sc, unsigned ub, unsigned n, unsigned char *dst)
{
        int i;

        CLOSE_ENTRY(36, 12, fugue4_core)
        for (i = 0; i < 32; i ++) {
                ROR(3, 36);
                CMIX36(S[0], S[1], S[2], S[4], S[5], S[6], S[18], S[19], S[20]);
                SMIX(S[0], S[1], S[2], S[3]);
        }
        for (i = 0; i < 13; i ++) {
                S[4] ^= S[0];
                S[9] ^= S[0];
                S[18] ^= S[0];
                S[27] ^= S[0];
                ROR(9, 36);
                SMIX(S[0], S[1], S[2], S[3]);
                S[4] ^= S[0];
                S[10] ^= S[0];
                S[18] ^= S[0];
                S[27] ^= S[0];
                ROR(9, 36);
                SMIX(S[0], S[1], S[2], S[3]);
                S[4] ^= S[0];
                S[10] ^= S[0];
                S[19] ^= S[0];
                S[27] ^= S[0];
                ROR(9, 36);
                SMIX(S[0], S[1], S[2], S[3]);
                S[4] ^= S[0];
                S[10] ^= S[0];
                S[19] ^= S[0];
                S[28] ^= S[0];
                ROR(8, 36);
                SMIX(S[0], S[1], S[2], S[3]);
        }
        S[4] ^= S[0];
        S[9] ^= S[0];
        S[18] ^= S[0];
        S[27] ^= S[0];
    out = dst; // unsigned char *out;
        sph_enc32be(out +  0, S[ 1]);
        sph_enc32be(out +  4, S[ 2]);
        sph_enc32be(out +  8, S[ 3]);
        sph_enc32be(out + 12, S[ 4]);
        sph_enc32be(out + 16, S[ 9]);
        sph_enc32be(out + 20, S[10]);
        sph_enc32be(out + 24, S[11]);
        sph_enc32be(out + 28, S[12]);
        sph_enc32be(out + 32, S[18]);
        sph_enc32be(out + 36, S[19]);
        sph_enc32be(out + 40, S[20]);
        sph_enc32be(out + 44, S[21]);
        sph_enc32be(out + 48, S[27]);
        sph_enc32be(out + 52, S[28]);
        sph_enc32be(out + 56, S[29]);
        sph_enc32be(out + 60, S[30]);
        sph_fugue512_init(sc);
}


void
sph_fugue224_init(sph_fugue_context *cc)
{
        fugue_init(cc, 23, fugue_IV224, 7);
}

void
sph_fugue224(sph_fugue_context *cc, const unsigned char *data, size_t len)
{
        fugue2_core(cc, data, len);
}

void
sph_fugue224_close(sph_fugue_context *cc, unsigned char *dst)
{
        fugue2_close(cc, 0, 0, dst, 7);
}

void
sph_fugue224_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        fugue2_close(cc, ub, n, dst, 7);
}

void
sph_fugue256_init(sph_fugue_context *cc)
{
        fugue_init(cc, 22, fugue_IV256, 8);
}

void
sph_fugue256(sph_fugue_context *cc, const unsigned char *data, size_t len)
{
        fugue2_core(cc, data, len);
}

void
sph_fugue256_close(sph_fugue_context *cc, unsigned char *dst)
{
        fugue2_close(cc, 0, 0, dst, 8);
}

void
sph_fugue256_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        fugue2_close(cc, ub, n, dst, 8);
}

void
sph_fugue384_init(sph_fugue_context *cc)
{
        fugue_init(cc, 24, fugue_IV384, 12);
}

void
sph_fugue384(sph_fugue_context *cc, const unsigned char *data, size_t len)
{
        fugue3_core(cc, data, len);
}

void
sph_fugue384_close(sph_fugue_context *cc, unsigned char *dst)
{
        fugue3_close(cc, 0, 0, dst);
}

void
sph_fugue384_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        fugue3_close(cc, ub, n, dst);
}

void
sph_fugue512_init(sph_fugue_context *cc)
{
        fugue_init(cc, 20, fugue_IV512, 16);
}

void
sph_fugue512(sph_fugue_context *cc, const unsigned char *data, size_t len)
{
        fugue4_core(cc, data, len);
}

void
sph_fugue512_close(sph_fugue_context *cc, unsigned char *dst)
{
        fugue4_close(cc, 0, 0, dst);
}

void
sph_fugue512_addbits_and_close(sph_fugue_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        fugue4_close(cc, ub, n, dst);
}












#define SPH_SIZE_keccak224   224
#define SPH_SIZE_keccak256   256
#define SPH_SIZE_keccak384   384
#define SPH_SIZE_keccak512   512

typedef struct {
        unsigned char buf[144];    /* first field, for alignment */
        size_t ptr, lim;
        union {
                sph_u64 wide[25];
                sph_u32 narrow[50];
        } u;
} sph_keccak_context;

void
sph_keccak224_init(sph_keccak_context *cc);
void
sph_keccak224(sph_keccak_context *cc, const unsigned char *data, size_t len);
void
sph_keccak224_close(sph_keccak_context *cc, unsigned char *dst);
void
sph_keccak224_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_keccak256_init(sph_keccak_context *cc);
void
sph_keccak256(sph_keccak_context *cc, const unsigned char *data, size_t len);
void
sph_keccak256_close(sph_keccak_context *cc, unsigned char *dst);
void
sph_keccak256_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_keccak384_init(sph_keccak_context *cc);
void
sph_keccak384(sph_keccak_context *cc, const unsigned char *data, size_t len);
void
sph_keccak384_close(sph_keccak_context *cc, unsigned char *dst);
void
sph_keccak384_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_keccak512_init(sph_keccak_context *cc);
void
sph_keccak512(sph_keccak_context *cc, const unsigned char *data, size_t len);
void
sph_keccak512_close(sph_keccak_context *cc, unsigned char *dst);
void
sph_keccak512_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst);











#define SPH_KECCAK_64   1
#define SPH_KECCAK_UNROLL   8
#define SPH_KECCAK_NOCOPY   1


static const sph_u64 RC[] = {
        SPH_C64(0x0000000000000001), SPH_C64(0x0000000000008082),
        SPH_C64(0x800000000000808A), SPH_C64(0x8000000080008000),
        SPH_C64(0x000000000000808B), SPH_C64(0x0000000080000001),
        SPH_C64(0x8000000080008081), SPH_C64(0x8000000000008009),
        SPH_C64(0x000000000000008A), SPH_C64(0x0000000000000088),
        SPH_C64(0x0000000080008009), SPH_C64(0x000000008000000A),
        SPH_C64(0x000000008000808B), SPH_C64(0x800000000000008B),
        SPH_C64(0x8000000000008089), SPH_C64(0x8000000000008003),
        SPH_C64(0x8000000000008002), SPH_C64(0x8000000000000080),
        SPH_C64(0x000000000000800A), SPH_C64(0x800000008000000A),
        SPH_C64(0x8000000080008081), SPH_C64(0x8000000000008080),
        SPH_C64(0x0000000080000001), SPH_C64(0x8000000080008008)
};

#define a00   (kc->u.wide[ 0])
#define a10   (kc->u.wide[ 1])
#define a20   (kc->u.wide[ 2])
#define a30   (kc->u.wide[ 3])
#define a40   (kc->u.wide[ 4])
#define a01   (kc->u.wide[ 5])
#define a11   (kc->u.wide[ 6])
#define a21   (kc->u.wide[ 7])
#define a31   (kc->u.wide[ 8])
#define a41   (kc->u.wide[ 9])
#define a02   (kc->u.wide[10])
#define a12   (kc->u.wide[11])
#define a22   (kc->u.wide[12])
#define a32   (kc->u.wide[13])
#define a42   (kc->u.wide[14])
#define a03   (kc->u.wide[15])
#define a13   (kc->u.wide[16])
#define a23   (kc->u.wide[17])
#define a33   (kc->u.wide[18])
#define a43   (kc->u.wide[19])
#define a04   (kc->u.wide[20])
#define a14   (kc->u.wide[21])
#define a24   (kc->u.wide[22])
#define a34   (kc->u.wide[23])
#define a44   (kc->u.wide[24])

#define keccak_DECL_STATE
#define keccak_READ_STATE(sc)
#define keccak_WRITE_STATE(sc)

#define INPUT_BUF(size)   do { \
                size_t j; \
                for (j = 0; j < (size); j += 8) { \
                        kc->u.wide[j >> 3] ^= sph_dec64le_aligned(buf + j); \
                } \
        } while (0)

#define INPUT_BUF144   INPUT_BUF(144)
#define INPUT_BUF136   INPUT_BUF(136)
#define INPUT_BUF104   INPUT_BUF(104)
#define INPUT_BUF72    INPUT_BUF(72)


#define DECL64(x)        sph_u64 x
#define MOV64(d, s)      (d = s)
#define XOR64(d, a, b)   (d = a ^ b)
#define AND64(d, a, b)   (d = a & b)
#define OR64(d, a, b)    (d = a | b)
#define NOT64(d, s)      (d = SPH_T64(~s))
#define ROL64(d, v, n)   (d = SPH_ROTL64(v, n))


#define TH_ELT(t, c0, c1, c2, c3, c4, d0, d1, d2, d3, d4)   do { \
                DECL64(tt0); \
                DECL64(tt1); \
                DECL64(tt2); \
                DECL64(tt3); \
                XOR64(tt0, d0, d1); \
                XOR64(tt1, d2, d3); \
                XOR64(tt0, tt0, d4); \
                XOR64(tt0, tt0, tt1); \
                ROL64(tt0, tt0, 1); \
                XOR64(tt2, c0, c1); \
                XOR64(tt3, c2, c3); \
                XOR64(tt0, tt0, c4); \
                XOR64(tt2, tt2, tt3); \
                XOR64(t, tt0, tt2); \
        } while (0)




#define KHI_XO(d, a, b, c)   do { \
                DECL64(kt); \
                OR64(kt, b, c); \
                XOR64(d, a, kt); \
        } while (0)

#define KHI_XA(d, a, b, c)   do { \
                DECL64(kt); \
                AND64(kt, b, c); \
                XOR64(d, a, kt); \
        } while (0)



#define P0    a00, a01, a02, a03, a04, a10, a11, a12, a13, a14, a20, a21, \
              a22, a23, a24, a30, a31, a32, a33, a34, a40, a41, a42, a43, a44
#define P1    a00, a30, a10, a40, a20, a11, a41, a21, a01, a31, a22, a02, \
              a32, a12, a42, a33, a13, a43, a23, a03, a44, a24, a04, a34, a14
#define P2    a00, a33, a11, a44, a22, a41, a24, a02, a30, a13, a32, a10, \
              a43, a21, a04, a23, a01, a34, a12, a40, a14, a42, a20, a03, a31
#define keccak_P3    a00, a23, a41, a14, a32, a24, a42, a10, a33, a01, a43, a11, \
              a34, a02, a20, a12, a30, a03, a21, a44, a31, a04, a22, a40, a13
#define keccak_P4    a00, a12, a24, a31, a43, a42, a04, a11, a23, a30, a34, a41, \
              a03, a10, a22, a21, a33, a40, a02, a14, a13, a20, a32, a44, a01
#define keccak_P5    a00, a21, a42, a13, a34, a04, a20, a41, a12, a33, a03, a24, \
              a40, a11, a32, a02, a23, a44, a10, a31, a01, a22, a43, a14, a30
#define P6    a00, a02, a04, a01, a03, a20, a22, a24, a21, a23, a40, a42, \
              a44, a41, a43, a10, a12, a14, a11, a13, a30, a32, a34, a31, a33
#define P7    a00, a10, a20, a30, a40, a22, a32, a42, a02, a12, a44, a04, \
              a14, a24, a34, a11, a21, a31, a41, a01, a33, a43, a03, a13, a23
#define P8    a00, a11, a22, a33, a44, a32, a43, a04, a10, a21, a14, a20, \
              a31, a42, a03, a41, a02, a13, a24, a30, a23, a34, a40, a01, a12
#define P9    a00, a41, a32, a23, a14, a43, a34, a20, a11, a02, a31, a22, \
              a13, a04, a40, a24, a10, a01, a42, a33, a12, a03, a44, a30, a21
#define P10   a00, a24, a43, a12, a31, a34, a03, a22, a41, a10, a13, a32, \
              a01, a20, a44, a42, a11, a30, a04, a23, a21, a40, a14, a33, a02
#define P11   a00, a42, a34, a21, a13, a03, a40, a32, a24, a11, a01, a43, \
              a30, a22, a14, a04, a41, a33, a20, a12, a02, a44, a31, a23, a10
#define P12   a00, a04, a03, a02, a01, a40, a44, a43, a42, a41, a30, a34, \
              a33, a32, a31, a20, a24, a23, a22, a21, a10, a14, a13, a12, a11
#define P13   a00, a20, a40, a10, a30, a44, a14, a34, a04, a24, a33, a03, \
              a23, a43, a13, a22, a42, a12, a32, a02, a11, a31, a01, a21, a41
#define P14   a00, a22, a44, a11, a33, a14, a31, a03, a20, a42, a23, a40, \
              a12, a34, a01, a32, a04, a21, a43, a10, a41, a13, a30, a02, a24
#define P15   a00, a32, a14, a41, a23, a31, a13, a40, a22, a04, a12, a44, \
              a21, a03, a30, a43, a20, a02, a34, a11, a24, a01, a33, a10, a42
#define P16   a00, a43, a31, a24, a12, a13, a01, a44, a32, a20, a21, a14, \
              a02, a40, a33, a34, a22, a10, a03, a41, a42, a30, a23, a11, a04
#define P17   a00, a34, a13, a42, a21, a01, a30, a14, a43, a22, a02, a31, \
              a10, a44, a23, a03, a32, a11, a40, a24, a04, a33, a12, a41, a20
#define P18   a00, a03, a01, a04, a02, a30, a33, a31, a34, a32, a10, a13, \
              a11, a14, a12, a40, a43, a41, a44, a42, a20, a23, a21, a24, a22
#define P19   a00, a40, a30, a20, a10, a33, a23, a13, a03, a43, a11, a01, \
              a41, a31, a21, a44, a34, a24, a14, a04, a22, a12, a02, a42, a32
#define P20   a00, a44, a33, a22, a11, a23, a12, a01, a40, a34, a41, a30, \
              a24, a13, a02, a14, a03, a42, a31, a20, a32, a21, a10, a04, a43
#define P21   a00, a14, a23, a32, a41, a12, a21, a30, a44, a03, a24, a33, \
              a42, a01, a10, a31, a40, a04, a13, a22, a43, a02, a11, a20, a34
#define P22   a00, a31, a12, a43, a24, a21, a02, a33, a14, a40, a42, a23, \
              a04, a30, a11, a13, a44, a20, a01, a32, a34, a10, a41, a22, a03
#define P23   a00, a13, a21, a34, a42, a02, a10, a23, a31, a44, a04, a12, \
              a20, a33, a41, a01, a14, a22, a30, a43, a03, a11, a24, a32, a40

#define P1_TO_P0   do { \
                DECL64(t); \
                MOV64(t, a01); \
                MOV64(a01, a30); \
                MOV64(a30, a33); \
                MOV64(a33, a23); \
                MOV64(a23, a12); \
                MOV64(a12, a21); \
                MOV64(a21, a02); \
                MOV64(a02, a10); \
                MOV64(a10, a11); \
                MOV64(a11, a41); \
                MOV64(a41, a24); \
                MOV64(a24, a42); \
                MOV64(a42, a04); \
                MOV64(a04, a20); \
                MOV64(a20, a22); \
                MOV64(a22, a32); \
                MOV64(a32, a43); \
                MOV64(a43, a34); \
                MOV64(a34, a03); \
                MOV64(a03, a40); \
                MOV64(a40, a44); \
                MOV64(a44, a14); \
                MOV64(a14, a31); \
                MOV64(a31, a13); \
                MOV64(a13, t); \
        } while (0)

#define P2_TO_P0   do { \
                DECL64(t); \
                MOV64(t, a01); \
                MOV64(a01, a33); \
                MOV64(a33, a12); \
                MOV64(a12, a02); \
                MOV64(a02, a11); \
                MOV64(a11, a24); \
                MOV64(a24, a04); \
                MOV64(a04, a22); \
                MOV64(a22, a43); \
                MOV64(a43, a03); \
                MOV64(a03, a44); \
                MOV64(a44, a31); \
                MOV64(a31, t); \
                MOV64(t, a10); \
                MOV64(a10, a41); \
                MOV64(a41, a42); \
                MOV64(a42, a20); \
                MOV64(a20, a32); \
                MOV64(a32, a34); \
                MOV64(a34, a40); \
                MOV64(a40, a14); \
                MOV64(a14, a13); \
                MOV64(a13, a30); \
                MOV64(a30, a23); \
                MOV64(a23, a21); \
                MOV64(a21, t); \
        } while (0)

#define P4_TO_P0   do { \
                DECL64(t); \
                MOV64(t, a01); \
                MOV64(a01, a12); \
                MOV64(a12, a11); \
                MOV64(a11, a04); \
                MOV64(a04, a43); \
                MOV64(a43, a44); \
                MOV64(a44, t); \
                MOV64(t, a02); \
                MOV64(a02, a24); \
                MOV64(a24, a22); \
                MOV64(a22, a03); \
                MOV64(a03, a31); \
                MOV64(a31, a33); \
                MOV64(a33, t); \
                MOV64(t, a10); \
                MOV64(a10, a42); \
                MOV64(a42, a32); \
                MOV64(a32, a40); \
                MOV64(a40, a13); \
                MOV64(a13, a23); \
                MOV64(a23, t); \
                MOV64(t, a14); \
                MOV64(a14, a30); \
                MOV64(a30, a21); \
                MOV64(a21, a41); \
                MOV64(a41, a20); \
                MOV64(a20, a34); \
                MOV64(a34, t); \
        } while (0)

#define P6_TO_P0   do { \
                DECL64(t); \
                MOV64(t, a01); \
                MOV64(a01, a02); \
                MOV64(a02, a04); \
                MOV64(a04, a03); \
                MOV64(a03, t); \
                MOV64(t, a10); \
                MOV64(a10, a20); \
                MOV64(a20, a40); \
                MOV64(a40, a30); \
                MOV64(a30, t); \
                MOV64(t, a11); \
                MOV64(a11, a22); \
                MOV64(a22, a44); \
                MOV64(a44, a33); \
                MOV64(a33, t); \
                MOV64(t, a12); \
                MOV64(a12, a24); \
                MOV64(a24, a43); \
                MOV64(a43, a31); \
                MOV64(a31, t); \
                MOV64(t, a13); \
                MOV64(a13, a21); \
                MOV64(a21, a42); \
                MOV64(a42, a34); \
                MOV64(a34, t); \
                MOV64(t, a14); \
                MOV64(a14, a23); \
                MOV64(a23, a41); \
                MOV64(a41, a32); \
                MOV64(a32, t); \
        } while (0)

#define P8_TO_P0   do { \
                DECL64(t); \
                MOV64(t, a01); \
                MOV64(a01, a11); \
                MOV64(a11, a43); \
                MOV64(a43, t); \
                MOV64(t, a02); \
                MOV64(a02, a22); \
                MOV64(a22, a31); \
                MOV64(a31, t); \
                MOV64(t, a03); \
                MOV64(a03, a33); \
                MOV64(a33, a24); \
                MOV64(a24, t); \
                MOV64(t, a04); \
                MOV64(a04, a44); \
                MOV64(a44, a12); \
                MOV64(a12, t); \
                MOV64(t, a10); \
                MOV64(a10, a32); \
                MOV64(a32, a13); \
                MOV64(a13, t); \
                MOV64(t, a14); \
                MOV64(a14, a21); \
                MOV64(a21, a20); \
                MOV64(a20, t); \
                MOV64(t, a23); \
                MOV64(a23, a42); \
                MOV64(a42, a40); \
                MOV64(a40, t); \
                MOV64(t, a30); \
                MOV64(a30, a41); \
                MOV64(a41, a34); \
                MOV64(a34, t); \
        } while (0)

#define P12_TO_P0   do { \
                DECL64(t); \
                MOV64(t, a01); \
                MOV64(a01, a04); \
                MOV64(a04, t); \
                MOV64(t, a02); \
                MOV64(a02, a03); \
                MOV64(a03, t); \
                MOV64(t, a10); \
                MOV64(a10, a40); \
                MOV64(a40, t); \
                MOV64(t, a11); \
                MOV64(a11, a44); \
                MOV64(a44, t); \
                MOV64(t, a12); \
                MOV64(a12, a43); \
                MOV64(a43, t); \
                MOV64(t, a13); \
                MOV64(a13, a42); \
                MOV64(a42, t); \
                MOV64(t, a14); \
                MOV64(a14, a41); \
                MOV64(a41, t); \
                MOV64(t, a20); \
                MOV64(a20, a30); \
                MOV64(a30, t); \
                MOV64(t, a21); \
                MOV64(a21, a34); \
                MOV64(a34, t); \
                MOV64(t, a22); \
                MOV64(a22, a33); \
                MOV64(a33, t); \
                MOV64(t, a23); \
                MOV64(a23, a32); \
                MOV64(a32, t); \
                MOV64(t, a24); \
                MOV64(a24, a31); \
                MOV64(a31, t); \
        } while (0)

#define LPAR   (
#define RPAR   )



static void
keccak_init(sph_keccak_context *kc, unsigned out_size)
{
        int i;

        for (i = 0; i < 25; i ++)
                kc->u.wide[i] = 0;
        /*
         * Initialization for the "lane complement".
         */
        kc->u.wide[ 1] = SPH_C64(0xFFFFFFFFFFFFFFFF);
        kc->u.wide[ 2] = SPH_C64(0xFFFFFFFFFFFFFFFF);
        kc->u.wide[ 8] = SPH_C64(0xFFFFFFFFFFFFFFFF);
        kc->u.wide[12] = SPH_C64(0xFFFFFFFFFFFFFFFF);
        kc->u.wide[17] = SPH_C64(0xFFFFFFFFFFFFFFFF);
        kc->u.wide[20] = SPH_C64(0xFFFFFFFFFFFFFFFF);
        kc->ptr = 0;
        kc->lim = 200 - (out_size >> 2);
}

static void
keccak_core(sph_keccak_context *kc, const void *data, size_t len, size_t lim)
{
        unsigned char *buf;
        size_t ptr;
        keccak_DECL_STATE

        buf = kc->buf;
        ptr = kc->ptr;

        if (len < (lim - ptr)) {
                memcpy(buf + ptr, data, len);
                kc->ptr = ptr + len;
                return;
        }

        keccak_READ_STATE(kc);
        while (len > 0) {
                size_t clen;

                clen = (lim - ptr);
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                ptr += clen;
                data = (const unsigned char *)data + clen;
                len -= clen;
                if (ptr == lim) {
                        INPUT_BUF(lim);
                        // KECCAK_F_1600;
    do {
      int j;
      for (j = 0; j < 24; j += 8) {
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[1]) ^ (kc->u.wide[6]));
              (tt1 = (kc->u.wide[11]) ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ (kc->u.wide[21]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[4]) ^ (kc->u.wide[9]));
              (tt3 = (kc->u.wide[14]) ^ (kc->u.wide[19]));
              (tt0 = tt0 ^ (kc->u.wide[24]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[2]) ^ (kc->u.wide[7]));
              (tt1 = (kc->u.wide[12]) ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ (kc->u.wide[22]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[5]));
              (tt3 = (kc->u.wide[10]) ^ (kc->u.wide[15]));
              (tt0 = tt0 ^ (kc->u.wide[20]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[3]) ^ (kc->u.wide[8]));
              (tt1 = (kc->u.wide[13]) ^ (kc->u.wide[18]));
              (tt0 = tt0 ^ (kc->u.wide[23]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[1]) ^ (kc->u.wide[6]));
              (tt3 = (kc->u.wide[11]) ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ (kc->u.wide[21]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[4]) ^ (kc->u.wide[9]));
              (tt1 = (kc->u.wide[14]) ^ (kc->u.wide[19]));
              (tt0 = tt0 ^ (kc->u.wide[24]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[2]) ^ (kc->u.wide[7]));
              (tt3 = (kc->u.wide[12]) ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ (kc->u.wide[22]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[5]));
              (tt1 = (kc->u.wide[10]) ^ (kc->u.wide[15]));
              (tt0 = tt0 ^ (kc->u.wide[20]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[3]) ^ (kc->u.wide[8]));
              (tt3 = (kc->u.wide[13]) ^ (kc->u.wide[18]));
              (tt0 = tt0 ^ (kc->u.wide[23]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t0);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t0);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t0);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t0);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t1);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t1);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t1);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t1);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t1);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t2);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t2);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t2);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t2);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t2);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t3);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t3);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t3);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t3);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t3);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t4);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t4);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t4);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t4);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (36)) |
                                 (((kc->u.wide[5])) >> (64 - (36)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (3)) |
                                  (((kc->u.wide[10])) >> (64 - (3)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (41)) |
                                  (((kc->u.wide[15])) >> (64 - (41)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (18)) |
                                  (((kc->u.wide[20])) >> (64 - (18)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (1)) |
                                 (((kc->u.wide[1])) >> (64 - (1)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (44)) |
                                 (((kc->u.wide[6])) >> (64 - (44)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (10)) |
                                  (((kc->u.wide[11])) >> (64 - (10)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (45)) |
                                  (((kc->u.wide[16])) >> (64 - (45)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (2)) |
                                  (((kc->u.wide[21])) >> (64 - (2)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (62)) |
                                 (((kc->u.wide[2])) >> (64 - (62)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (6)) |
                                 (((kc->u.wide[7])) >> (64 - (6)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (43)) |
                                  (((kc->u.wide[12])) >> (64 - (43)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (15)) |
                                  (((kc->u.wide[17])) >> (64 - (15)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (61)) |
                                  (((kc->u.wide[22])) >> (64 - (61)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (28)) |
                                 (((kc->u.wide[3])) >> (64 - (28)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (55)) |
                                 (((kc->u.wide[8])) >> (64 - (55)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (25)) |
                                  (((kc->u.wide[13])) >> (64 - (25)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (21)) |
                                  (((kc->u.wide[18])) >> (64 - (21)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (56)) |
                                  (((kc->u.wide[23])) >> (64 - (56)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (27)) |
                                 (((kc->u.wide[4])) >> (64 - (27)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (20)) |
                                 (((kc->u.wide[9])) >> (64 - (20)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (39)) |
                                  (((kc->u.wide[14])) >> (64 - (39)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (8)) |
                                  (((kc->u.wide[19])) >> (64 - (8)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (14)) |
                                  (((kc->u.wide[24])) >> (64 - (14)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[12])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[6]) | (kc->u.wide[12]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[18]));
              (c1 = (kc->u.wide[6]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[18]) & (kc->u.wide[24]));
              (c2 = (kc->u.wide[12]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[24]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[18]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[6]));
              (c4 = (kc->u.wide[24]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[6]) = c1);
            ((kc->u.wide[12]) = c2);
            ((kc->u.wide[18]) = c3);
            ((kc->u.wide[24]) = c4);
            (bnn = ((~(kc->u.wide[22])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[9]) | (kc->u.wide[10]));
              (c0 = (kc->u.wide[3]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[10]) & (kc->u.wide[16]));
              (c1 = (kc->u.wide[9]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[16]) | bnn);
              (c2 = (kc->u.wide[10]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[22]) | (kc->u.wide[3]));
              (c3 = (kc->u.wide[16]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[3]) & (kc->u.wide[9]));
              (c4 = (kc->u.wide[22]) ^ kt);
            } while (0);
            ((kc->u.wide[3]) = c0);
            ((kc->u.wide[9]) = c1);
            ((kc->u.wide[10]) = c2);
            ((kc->u.wide[16]) = c3);
            ((kc->u.wide[22]) = c4);
            (bnn = ((~(kc->u.wide[19])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[7]) | (kc->u.wide[13]));
              (c0 = (kc->u.wide[1]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[13]) & (kc->u.wide[19]));
              (c1 = (kc->u.wide[7]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[20]));
              (c2 = (kc->u.wide[13]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) | (kc->u.wide[1]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[1]) & (kc->u.wide[7]));
              (c4 = (kc->u.wide[20]) ^ kt);
            } while (0);
            ((kc->u.wide[1]) = c0);
            ((kc->u.wide[7]) = c1);
            ((kc->u.wide[13]) = c2);
            ((kc->u.wide[19]) = c3);
            ((kc->u.wide[20]) = c4);
            (bnn = ((~(kc->u.wide[17])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[5]) & (kc->u.wide[11]));
              (c0 = (kc->u.wide[4]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[11]) | (kc->u.wide[17]));
              (c1 = (kc->u.wide[5]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[23]));
              (c2 = (kc->u.wide[11]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[23]) & (kc->u.wide[4]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[4]) | (kc->u.wide[5]));
              (c4 = (kc->u.wide[23]) ^ kt);
            } while (0);
            ((kc->u.wide[4]) = c0);
            ((kc->u.wide[5]) = c1);
            ((kc->u.wide[11]) = c2);
            ((kc->u.wide[17]) = c3);
            ((kc->u.wide[23]) = c4);
            (bnn = ((~(kc->u.wide[8])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[14]));
              (c0 = (kc->u.wide[2]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) | (kc->u.wide[15]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[15]) & (kc->u.wide[21]));
              (c2 = (kc->u.wide[14]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[21]) | (kc->u.wide[2]));
              (c3 = (kc->u.wide[15]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) & (kc->u.wide[8]));
              (c4 = (kc->u.wide[21]) ^ kt);
            } while (0);
            ((kc->u.wide[2]) = c0);
            ((kc->u.wide[8]) = c1);
            ((kc->u.wide[14]) = c2);
            ((kc->u.wide[15]) = c3);
            ((kc->u.wide[21]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 0]);
        } while (0);
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[6]) ^ (kc->u.wide[9]));
              (tt1 = (kc->u.wide[7]) ^ (kc->u.wide[5]));
              (tt0 = tt0 ^ (kc->u.wide[8]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[24]) ^ (kc->u.wide[22]));
              (tt3 = (kc->u.wide[20]) ^ (kc->u.wide[23]));
              (tt0 = tt0 ^ (kc->u.wide[21]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[12]) ^ (kc->u.wide[10]));
              (tt1 = (kc->u.wide[13]) ^ (kc->u.wide[11]));
              (tt0 = tt0 ^ (kc->u.wide[14]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[3]));
              (tt3 = (kc->u.wide[1]) ^ (kc->u.wide[4]));
              (tt0 = tt0 ^ (kc->u.wide[2]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[18]) ^ (kc->u.wide[16]));
              (tt1 = (kc->u.wide[19]) ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ (kc->u.wide[15]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[6]) ^ (kc->u.wide[9]));
              (tt3 = (kc->u.wide[7]) ^ (kc->u.wide[5]));
              (tt0 = tt0 ^ (kc->u.wide[8]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[24]) ^ (kc->u.wide[22]));
              (tt1 = (kc->u.wide[20]) ^ (kc->u.wide[23]));
              (tt0 = tt0 ^ (kc->u.wide[21]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[12]) ^ (kc->u.wide[10]));
              (tt3 = (kc->u.wide[13]) ^ (kc->u.wide[11]));
              (tt0 = tt0 ^ (kc->u.wide[14]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[3]));
              (tt1 = (kc->u.wide[1]) ^ (kc->u.wide[4]));
              (tt0 = tt0 ^ (kc->u.wide[2]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[18]) ^ (kc->u.wide[16]));
              (tt3 = (kc->u.wide[19]) ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ (kc->u.wide[15]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t0);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t0);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t0);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t0);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t1);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t1);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t1);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t1);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t1);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t2);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t2);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t2);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t2);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t2);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t3);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t3);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t3);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t3);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t3);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t4);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t4);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t4);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t4);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (36)) |
                                 (((kc->u.wide[3])) >> (64 - (36)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (3)) |
                                 (((kc->u.wide[1])) >> (64 - (3)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (41)) |
                                 (((kc->u.wide[4])) >> (64 - (41)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (18)) |
                                 (((kc->u.wide[2])) >> (64 - (18)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (1)) |
                                 (((kc->u.wide[6])) >> (64 - (1)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (44)) |
                                 (((kc->u.wide[9])) >> (64 - (44)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (10)) |
                                 (((kc->u.wide[7])) >> (64 - (10)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (45)) |
                                 (((kc->u.wide[5])) >> (64 - (45)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (2)) |
                                 (((kc->u.wide[8])) >> (64 - (2)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (62)) |
                                  (((kc->u.wide[12])) >> (64 - (62)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (6)) |
                                  (((kc->u.wide[10])) >> (64 - (6)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (43)) |
                                  (((kc->u.wide[13])) >> (64 - (43)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (15)) |
                                  (((kc->u.wide[11])) >> (64 - (15)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (61)) |
                                  (((kc->u.wide[14])) >> (64 - (61)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (28)) |
                                  (((kc->u.wide[18])) >> (64 - (28)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (55)) |
                                  (((kc->u.wide[16])) >> (64 - (55)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (25)) |
                                  (((kc->u.wide[19])) >> (64 - (25)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (21)) |
                                  (((kc->u.wide[17])) >> (64 - (21)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (56)) |
                                  (((kc->u.wide[15])) >> (64 - (56)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (27)) |
                                  (((kc->u.wide[24])) >> (64 - (27)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (20)) |
                                  (((kc->u.wide[22])) >> (64 - (20)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (39)) |
                                  (((kc->u.wide[20])) >> (64 - (39)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (8)) |
                                  (((kc->u.wide[23])) >> (64 - (8)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (14)) |
                                  (((kc->u.wide[21])) >> (64 - (14)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[13])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[9]) | (kc->u.wide[13]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[17]));
              (c1 = (kc->u.wide[9]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[17]) & (kc->u.wide[21]));
              (c2 = (kc->u.wide[13]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[21]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[17]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[9]));
              (c4 = (kc->u.wide[21]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[9]) = c1);
            ((kc->u.wide[13]) = c2);
            ((kc->u.wide[17]) = c3);
            ((kc->u.wide[21]) = c4);
            (bnn = ((~(kc->u.wide[14])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[22]) | (kc->u.wide[1]));
              (c0 = (kc->u.wide[18]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[1]) & (kc->u.wide[5]));
              (c1 = (kc->u.wide[22]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[5]) | bnn);
              (c2 = (kc->u.wide[1]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) | (kc->u.wide[18]));
              (c3 = (kc->u.wide[5]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[18]) & (kc->u.wide[22]));
              (c4 = (kc->u.wide[14]) ^ kt);
            } while (0);
            ((kc->u.wide[18]) = c0);
            ((kc->u.wide[22]) = c1);
            ((kc->u.wide[1]) = c2);
            ((kc->u.wide[5]) = c3);
            ((kc->u.wide[14]) = c4);
            (bnn = ((~(kc->u.wide[23])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[10]) | (kc->u.wide[19]));
              (c0 = (kc->u.wide[6]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[19]) & (kc->u.wide[23]));
              (c1 = (kc->u.wide[10]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[2]));
              (c2 = (kc->u.wide[19]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) | (kc->u.wide[6]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[6]) & (kc->u.wide[10]));
              (c4 = (kc->u.wide[2]) ^ kt);
            } while (0);
            ((kc->u.wide[6]) = c0);
            ((kc->u.wide[10]) = c1);
            ((kc->u.wide[19]) = c2);
            ((kc->u.wide[23]) = c3);
            ((kc->u.wide[2]) = c4);
            (bnn = ((~(kc->u.wide[11])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[3]) & (kc->u.wide[7]));
              (c0 = (kc->u.wide[24]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[7]) | (kc->u.wide[11]));
              (c1 = (kc->u.wide[3]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[15]));
              (c2 = (kc->u.wide[7]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[15]) & (kc->u.wide[24]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[24]) | (kc->u.wide[3]));
              (c4 = (kc->u.wide[15]) ^ kt);
            } while (0);
            ((kc->u.wide[24]) = c0);
            ((kc->u.wide[3]) = c1);
            ((kc->u.wide[7]) = c2);
            ((kc->u.wide[11]) = c3);
            ((kc->u.wide[15]) = c4);
            (bnn = ((~(kc->u.wide[16])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[20]));
              (c0 = (kc->u.wide[12]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) | (kc->u.wide[4]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[4]) & (kc->u.wide[8]));
              (c2 = (kc->u.wide[20]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[8]) | (kc->u.wide[12]));
              (c3 = (kc->u.wide[4]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[12]) & (kc->u.wide[16]));
              (c4 = (kc->u.wide[8]) ^ kt);
            } while (0);
            ((kc->u.wide[12]) = c0);
            ((kc->u.wide[16]) = c1);
            ((kc->u.wide[20]) = c2);
            ((kc->u.wide[4]) = c3);
            ((kc->u.wide[8]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 1]);
        } while (0);
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[9]) ^ (kc->u.wide[22]));
              (tt1 = (kc->u.wide[10]) ^ (kc->u.wide[3]));
              (tt0 = tt0 ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[21]) ^ (kc->u.wide[14]));
              (tt3 = (kc->u.wide[2]) ^ (kc->u.wide[15]));
              (tt0 = tt0 ^ (kc->u.wide[8]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[13]) ^ (kc->u.wide[1]));
              (tt1 = (kc->u.wide[19]) ^ (kc->u.wide[7]));
              (tt0 = tt0 ^ (kc->u.wide[20]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[18]));
              (tt3 = (kc->u.wide[6]) ^ (kc->u.wide[24]));
              (tt0 = tt0 ^ (kc->u.wide[12]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[17]) ^ (kc->u.wide[5]));
              (tt1 = (kc->u.wide[23]) ^ (kc->u.wide[11]));
              (tt0 = tt0 ^ (kc->u.wide[4]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[9]) ^ (kc->u.wide[22]));
              (tt3 = (kc->u.wide[10]) ^ (kc->u.wide[3]));
              (tt0 = tt0 ^ (kc->u.wide[16]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[21]) ^ (kc->u.wide[14]));
              (tt1 = (kc->u.wide[2]) ^ (kc->u.wide[15]));
              (tt0 = tt0 ^ (kc->u.wide[8]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[13]) ^ (kc->u.wide[1]));
              (tt3 = (kc->u.wide[19]) ^ (kc->u.wide[7]));
              (tt0 = tt0 ^ (kc->u.wide[20]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[18]));
              (tt1 = (kc->u.wide[6]) ^ (kc->u.wide[24]));
              (tt0 = tt0 ^ (kc->u.wide[12]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[17]) ^ (kc->u.wide[5]));
              (tt3 = (kc->u.wide[23]) ^ (kc->u.wide[11]));
              (tt0 = tt0 ^ (kc->u.wide[4]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t0);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t0);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t0);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t0);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t1);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t1);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t1);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t1);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t1);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t2);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t2);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t2);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t2);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t2);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t3);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t3);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t3);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t3);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t3);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t4);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t4);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t4);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t4);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (36)) |
                                  (((kc->u.wide[18])) >> (64 - (36)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (3)) |
                                 (((kc->u.wide[6])) >> (64 - (3)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (41)) |
                                  (((kc->u.wide[24])) >> (64 - (41)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (18)) |
                                  (((kc->u.wide[12])) >> (64 - (18)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (1)) |
                                 (((kc->u.wide[9])) >> (64 - (1)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (44)) |
                                  (((kc->u.wide[22])) >> (64 - (44)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (10)) |
                                  (((kc->u.wide[10])) >> (64 - (10)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (45)) |
                                 (((kc->u.wide[3])) >> (64 - (45)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (2)) |
                                  (((kc->u.wide[16])) >> (64 - (2)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (62)) |
                                  (((kc->u.wide[13])) >> (64 - (62)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (6)) |
                                 (((kc->u.wide[1])) >> (64 - (6)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (43)) |
                                  (((kc->u.wide[19])) >> (64 - (43)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (15)) |
                                 (((kc->u.wide[7])) >> (64 - (15)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (61)) |
                                  (((kc->u.wide[20])) >> (64 - (61)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (28)) |
                                  (((kc->u.wide[17])) >> (64 - (28)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (55)) |
                                 (((kc->u.wide[5])) >> (64 - (55)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (25)) |
                                  (((kc->u.wide[23])) >> (64 - (25)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (21)) |
                                  (((kc->u.wide[11])) >> (64 - (21)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (56)) |
                                 (((kc->u.wide[4])) >> (64 - (56)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (27)) |
                                  (((kc->u.wide[21])) >> (64 - (27)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (20)) |
                                  (((kc->u.wide[14])) >> (64 - (20)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (39)) |
                                 (((kc->u.wide[2])) >> (64 - (39)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (8)) |
                                  (((kc->u.wide[15])) >> (64 - (8)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (14)) |
                                 (((kc->u.wide[8])) >> (64 - (14)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[19])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[22]) | (kc->u.wide[19]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[11]));
              (c1 = (kc->u.wide[22]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[11]) & (kc->u.wide[8]));
              (c2 = (kc->u.wide[19]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[8]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[11]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[22]));
              (c4 = (kc->u.wide[8]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[22]) = c1);
            ((kc->u.wide[19]) = c2);
            ((kc->u.wide[11]) = c3);
            ((kc->u.wide[8]) = c4);
            (bnn = ((~(kc->u.wide[20])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) | (kc->u.wide[6]));
              (c0 = (kc->u.wide[17]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[6]) & (kc->u.wide[3]));
              (c1 = (kc->u.wide[14]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[3]) | bnn);
              (c2 = (kc->u.wide[6]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) | (kc->u.wide[17]));
              (c3 = (kc->u.wide[3]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[17]) & (kc->u.wide[14]));
              (c4 = (kc->u.wide[20]) ^ kt);
            } while (0);
            ((kc->u.wide[17]) = c0);
            ((kc->u.wide[14]) = c1);
            ((kc->u.wide[6]) = c2);
            ((kc->u.wide[3]) = c3);
            ((kc->u.wide[20]) = c4);
            (bnn = ((~(kc->u.wide[15])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[1]) | (kc->u.wide[23]));
              (c0 = (kc->u.wide[9]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[23]) & (kc->u.wide[15]));
              (c1 = (kc->u.wide[1]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[12]));
              (c2 = (kc->u.wide[23]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[12]) | (kc->u.wide[9]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[9]) & (kc->u.wide[1]));
              (c4 = (kc->u.wide[12]) ^ kt);
            } while (0);
            ((kc->u.wide[9]) = c0);
            ((kc->u.wide[1]) = c1);
            ((kc->u.wide[23]) = c2);
            ((kc->u.wide[15]) = c3);
            ((kc->u.wide[12]) = c4);
            (bnn = ((~(kc->u.wide[7])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[18]) & (kc->u.wide[10]));
              (c0 = (kc->u.wide[21]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[10]) | (kc->u.wide[7]));
              (c1 = (kc->u.wide[18]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[4]));
              (c2 = (kc->u.wide[10]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[4]) & (kc->u.wide[21]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[21]) | (kc->u.wide[18]));
              (c4 = (kc->u.wide[4]) ^ kt);
            } while (0);
            ((kc->u.wide[21]) = c0);
            ((kc->u.wide[18]) = c1);
            ((kc->u.wide[10]) = c2);
            ((kc->u.wide[7]) = c3);
            ((kc->u.wide[4]) = c4);
            (bnn = ((~(kc->u.wide[5])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[2]));
              (c0 = (kc->u.wide[13]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) | (kc->u.wide[24]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[24]) & (kc->u.wide[16]));
              (c2 = (kc->u.wide[2]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[16]) | (kc->u.wide[13]));
              (c3 = (kc->u.wide[24]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[13]) & (kc->u.wide[5]));
              (c4 = (kc->u.wide[16]) ^ kt);
            } while (0);
            ((kc->u.wide[13]) = c0);
            ((kc->u.wide[5]) = c1);
            ((kc->u.wide[2]) = c2);
            ((kc->u.wide[24]) = c3);
            ((kc->u.wide[16]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 2]);
        } while (0);
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[22]) ^ (kc->u.wide[14]));
              (tt1 = (kc->u.wide[1]) ^ (kc->u.wide[18]));
              (tt0 = tt0 ^ (kc->u.wide[5]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[8]) ^ (kc->u.wide[20]));
              (tt3 = (kc->u.wide[12]) ^ (kc->u.wide[4]));
              (tt0 = tt0 ^ (kc->u.wide[16]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[19]) ^ (kc->u.wide[6]));
              (tt1 = (kc->u.wide[23]) ^ (kc->u.wide[10]));
              (tt0 = tt0 ^ (kc->u.wide[2]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[17]));
              (tt3 = (kc->u.wide[9]) ^ (kc->u.wide[21]));
              (tt0 = tt0 ^ (kc->u.wide[13]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[11]) ^ (kc->u.wide[3]));
              (tt1 = (kc->u.wide[15]) ^ (kc->u.wide[7]));
              (tt0 = tt0 ^ (kc->u.wide[24]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[22]) ^ (kc->u.wide[14]));
              (tt3 = (kc->u.wide[1]) ^ (kc->u.wide[18]));
              (tt0 = tt0 ^ (kc->u.wide[5]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[8]) ^ (kc->u.wide[20]));
              (tt1 = (kc->u.wide[12]) ^ (kc->u.wide[4]));
              (tt0 = tt0 ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[19]) ^ (kc->u.wide[6]));
              (tt3 = (kc->u.wide[23]) ^ (kc->u.wide[10]));
              (tt0 = tt0 ^ (kc->u.wide[2]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[17]));
              (tt1 = (kc->u.wide[9]) ^ (kc->u.wide[21]));
              (tt0 = tt0 ^ (kc->u.wide[13]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[11]) ^ (kc->u.wide[3]));
              (tt3 = (kc->u.wide[15]) ^ (kc->u.wide[7]));
              (tt0 = tt0 ^ (kc->u.wide[24]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t0);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t0);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t0);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t0);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t1);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t1);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t1);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t1);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t1);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t2);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t2);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t2);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t2);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t2);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t3);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t3);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t3);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t3);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t3);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t4);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t4);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t4);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t4);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (36)) |
                                  (((kc->u.wide[17])) >> (64 - (36)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (3)) |
                                 (((kc->u.wide[9])) >> (64 - (3)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (41)) |
                                  (((kc->u.wide[21])) >> (64 - (41)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (18)) |
                                  (((kc->u.wide[13])) >> (64 - (18)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (1)) |
                                  (((kc->u.wide[22])) >> (64 - (1)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (44)) |
                                  (((kc->u.wide[14])) >> (64 - (44)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (10)) |
                                 (((kc->u.wide[1])) >> (64 - (10)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (45)) |
                                  (((kc->u.wide[18])) >> (64 - (45)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (2)) |
                                 (((kc->u.wide[5])) >> (64 - (2)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (62)) |
                                  (((kc->u.wide[19])) >> (64 - (62)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (6)) |
                                 (((kc->u.wide[6])) >> (64 - (6)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (43)) |
                                  (((kc->u.wide[23])) >> (64 - (43)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (15)) |
                                  (((kc->u.wide[10])) >> (64 - (15)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (61)) |
                                 (((kc->u.wide[2])) >> (64 - (61)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (28)) |
                                  (((kc->u.wide[11])) >> (64 - (28)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (55)) |
                                 (((kc->u.wide[3])) >> (64 - (55)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (25)) |
                                  (((kc->u.wide[15])) >> (64 - (25)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (21)) |
                                 (((kc->u.wide[7])) >> (64 - (21)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (56)) |
                                  (((kc->u.wide[24])) >> (64 - (56)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (27)) |
                                 (((kc->u.wide[8])) >> (64 - (27)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (20)) |
                                  (((kc->u.wide[20])) >> (64 - (20)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (39)) |
                                  (((kc->u.wide[12])) >> (64 - (39)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (8)) |
                                 (((kc->u.wide[4])) >> (64 - (8)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (14)) |
                                  (((kc->u.wide[16])) >> (64 - (14)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[23])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) | (kc->u.wide[23]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[7]));
              (c1 = (kc->u.wide[14]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[7]) & (kc->u.wide[16]));
              (c2 = (kc->u.wide[23]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[16]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[7]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[14]));
              (c4 = (kc->u.wide[16]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[14]) = c1);
            ((kc->u.wide[23]) = c2);
            ((kc->u.wide[7]) = c3);
            ((kc->u.wide[16]) = c4);
            (bnn = ((~(kc->u.wide[2])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) | (kc->u.wide[9]));
              (c0 = (kc->u.wide[11]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[9]) & (kc->u.wide[18]));
              (c1 = (kc->u.wide[20]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[18]) | bnn);
              (c2 = (kc->u.wide[9]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) | (kc->u.wide[11]));
              (c3 = (kc->u.wide[18]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[11]) & (kc->u.wide[20]));
              (c4 = (kc->u.wide[2]) ^ kt);
            } while (0);
            ((kc->u.wide[11]) = c0);
            ((kc->u.wide[20]) = c1);
            ((kc->u.wide[9]) = c2);
            ((kc->u.wide[18]) = c3);
            ((kc->u.wide[2]) = c4);
            (bnn = ((~(kc->u.wide[4])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[6]) | (kc->u.wide[15]));
              (c0 = (kc->u.wide[22]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[15]) & (kc->u.wide[4]));
              (c1 = (kc->u.wide[6]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[13]));
              (c2 = (kc->u.wide[15]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[13]) | (kc->u.wide[22]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[22]) & (kc->u.wide[6]));
              (c4 = (kc->u.wide[13]) ^ kt);
            } while (0);
            ((kc->u.wide[22]) = c0);
            ((kc->u.wide[6]) = c1);
            ((kc->u.wide[15]) = c2);
            ((kc->u.wide[4]) = c3);
            ((kc->u.wide[13]) = c4);
            (bnn = ((~(kc->u.wide[10])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[17]) & (kc->u.wide[1]));
              (c0 = (kc->u.wide[8]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[1]) | (kc->u.wide[10]));
              (c1 = (kc->u.wide[17]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[24]));
              (c2 = (kc->u.wide[1]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[24]) & (kc->u.wide[8]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[8]) | (kc->u.wide[17]));
              (c4 = (kc->u.wide[24]) ^ kt);
            } while (0);
            ((kc->u.wide[8]) = c0);
            ((kc->u.wide[17]) = c1);
            ((kc->u.wide[1]) = c2);
            ((kc->u.wide[10]) = c3);
            ((kc->u.wide[24]) = c4);
            (bnn = ((~(kc->u.wide[3])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[12]));
              (c0 = (kc->u.wide[19]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[12]) | (kc->u.wide[21]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[21]) & (kc->u.wide[5]));
              (c2 = (kc->u.wide[12]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[5]) | (kc->u.wide[19]));
              (c3 = (kc->u.wide[21]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[19]) & (kc->u.wide[3]));
              (c4 = (kc->u.wide[5]) ^ kt);
            } while (0);
            ((kc->u.wide[19]) = c0);
            ((kc->u.wide[3]) = c1);
            ((kc->u.wide[12]) = c2);
            ((kc->u.wide[21]) = c3);
            ((kc->u.wide[5]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 3]);
        } while (0);
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[14]) ^ (kc->u.wide[20]));
              (tt1 = (kc->u.wide[6]) ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ (kc->u.wide[3]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[16]) ^ (kc->u.wide[2]));
              (tt3 = (kc->u.wide[13]) ^ (kc->u.wide[24]));
              (tt0 = tt0 ^ (kc->u.wide[5]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[23]) ^ (kc->u.wide[9]));
              (tt1 = (kc->u.wide[15]) ^ (kc->u.wide[1]));
              (tt0 = tt0 ^ (kc->u.wide[12]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[11]));
              (tt3 = (kc->u.wide[22]) ^ (kc->u.wide[8]));
              (tt0 = tt0 ^ (kc->u.wide[19]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[7]) ^ (kc->u.wide[18]));
              (tt1 = (kc->u.wide[4]) ^ (kc->u.wide[10]));
              (tt0 = tt0 ^ (kc->u.wide[21]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[14]) ^ (kc->u.wide[20]));
              (tt3 = (kc->u.wide[6]) ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ (kc->u.wide[3]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[16]) ^ (kc->u.wide[2]));
              (tt1 = (kc->u.wide[13]) ^ (kc->u.wide[24]));
              (tt0 = tt0 ^ (kc->u.wide[5]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[23]) ^ (kc->u.wide[9]));
              (tt3 = (kc->u.wide[15]) ^ (kc->u.wide[1]));
              (tt0 = tt0 ^ (kc->u.wide[12]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[11]));
              (tt1 = (kc->u.wide[22]) ^ (kc->u.wide[8]));
              (tt0 = tt0 ^ (kc->u.wide[19]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[7]) ^ (kc->u.wide[18]));
              (tt3 = (kc->u.wide[4]) ^ (kc->u.wide[10]));
              (tt0 = tt0 ^ (kc->u.wide[21]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t0);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t0);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t0);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t0);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t1);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t1);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t1);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t1);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t1);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t2);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t2);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t2);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t2);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t2);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t3);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t3);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t3);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t3);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t3);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t4);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t4);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t4);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t4);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (36)) |
                                  (((kc->u.wide[11])) >> (64 - (36)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (3)) |
                                  (((kc->u.wide[22])) >> (64 - (3)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (41)) |
                                 (((kc->u.wide[8])) >> (64 - (41)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (18)) |
                                  (((kc->u.wide[19])) >> (64 - (18)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (1)) |
                                  (((kc->u.wide[14])) >> (64 - (1)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (44)) |
                                  (((kc->u.wide[20])) >> (64 - (44)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (10)) |
                                 (((kc->u.wide[6])) >> (64 - (10)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (45)) |
                                  (((kc->u.wide[17])) >> (64 - (45)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (2)) |
                                 (((kc->u.wide[3])) >> (64 - (2)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (62)) |
                                  (((kc->u.wide[23])) >> (64 - (62)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (6)) |
                                 (((kc->u.wide[9])) >> (64 - (6)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (43)) |
                                  (((kc->u.wide[15])) >> (64 - (43)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (15)) |
                                 (((kc->u.wide[1])) >> (64 - (15)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (61)) |
                                  (((kc->u.wide[12])) >> (64 - (61)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (28)) |
                                 (((kc->u.wide[7])) >> (64 - (28)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (55)) |
                                  (((kc->u.wide[18])) >> (64 - (55)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (25)) |
                                 (((kc->u.wide[4])) >> (64 - (25)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (21)) |
                                  (((kc->u.wide[10])) >> (64 - (21)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (56)) |
                                  (((kc->u.wide[21])) >> (64 - (56)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (27)) |
                                  (((kc->u.wide[16])) >> (64 - (27)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (20)) |
                                 (((kc->u.wide[2])) >> (64 - (20)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (39)) |
                                  (((kc->u.wide[13])) >> (64 - (39)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (8)) |
                                  (((kc->u.wide[24])) >> (64 - (8)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (14)) |
                                 (((kc->u.wide[5])) >> (64 - (14)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[15])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) | (kc->u.wide[15]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[10]));
              (c1 = (kc->u.wide[20]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[10]) & (kc->u.wide[5]));
              (c2 = (kc->u.wide[15]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[5]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[10]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[20]));
              (c4 = (kc->u.wide[5]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[20]) = c1);
            ((kc->u.wide[15]) = c2);
            ((kc->u.wide[10]) = c3);
            ((kc->u.wide[5]) = c4);
            (bnn = ((~(kc->u.wide[12])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) | (kc->u.wide[22]));
              (c0 = (kc->u.wide[7]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[22]) & (kc->u.wide[17]));
              (c1 = (kc->u.wide[2]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[17]) | bnn);
              (c2 = (kc->u.wide[22]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[12]) | (kc->u.wide[7]));
              (c3 = (kc->u.wide[17]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[7]) & (kc->u.wide[2]));
              (c4 = (kc->u.wide[12]) ^ kt);
            } while (0);
            ((kc->u.wide[7]) = c0);
            ((kc->u.wide[2]) = c1);
            ((kc->u.wide[22]) = c2);
            ((kc->u.wide[17]) = c3);
            ((kc->u.wide[12]) = c4);
            (bnn = ((~(kc->u.wide[24])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[9]) | (kc->u.wide[4]));
              (c0 = (kc->u.wide[14]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[4]) & (kc->u.wide[24]));
              (c1 = (kc->u.wide[9]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[19]));
              (c2 = (kc->u.wide[4]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[19]) | (kc->u.wide[14]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) & (kc->u.wide[9]));
              (c4 = (kc->u.wide[19]) ^ kt);
            } while (0);
            ((kc->u.wide[14]) = c0);
            ((kc->u.wide[9]) = c1);
            ((kc->u.wide[4]) = c2);
            ((kc->u.wide[24]) = c3);
            ((kc->u.wide[19]) = c4);
            (bnn = ((~(kc->u.wide[1])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[11]) & (kc->u.wide[6]));
              (c0 = (kc->u.wide[16]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[6]) | (kc->u.wide[1]));
              (c1 = (kc->u.wide[11]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[21]));
              (c2 = (kc->u.wide[6]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[21]) & (kc->u.wide[16]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[16]) | (kc->u.wide[11]));
              (c4 = (kc->u.wide[21]) ^ kt);
            } while (0);
            ((kc->u.wide[16]) = c0);
            ((kc->u.wide[11]) = c1);
            ((kc->u.wide[6]) = c2);
            ((kc->u.wide[1]) = c3);
            ((kc->u.wide[21]) = c4);
            (bnn = ((~(kc->u.wide[18])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[13]));
              (c0 = (kc->u.wide[23]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[13]) | (kc->u.wide[8]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[8]) & (kc->u.wide[3]));
              (c2 = (kc->u.wide[13]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[3]) | (kc->u.wide[23]));
              (c3 = (kc->u.wide[8]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[23]) & (kc->u.wide[18]));
              (c4 = (kc->u.wide[3]) ^ kt);
            } while (0);
            ((kc->u.wide[23]) = c0);
            ((kc->u.wide[18]) = c1);
            ((kc->u.wide[13]) = c2);
            ((kc->u.wide[8]) = c3);
            ((kc->u.wide[3]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 4]);
        } while (0);
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[20]) ^ (kc->u.wide[2]));
              (tt1 = (kc->u.wide[9]) ^ (kc->u.wide[11]));
              (tt0 = tt0 ^ (kc->u.wide[18]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[5]) ^ (kc->u.wide[12]));
              (tt3 = (kc->u.wide[19]) ^ (kc->u.wide[21]));
              (tt0 = tt0 ^ (kc->u.wide[3]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[15]) ^ (kc->u.wide[22]));
              (tt1 = (kc->u.wide[4]) ^ (kc->u.wide[6]));
              (tt0 = tt0 ^ (kc->u.wide[13]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[7]));
              (tt3 = (kc->u.wide[14]) ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ (kc->u.wide[23]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[10]) ^ (kc->u.wide[17]));
              (tt1 = (kc->u.wide[24]) ^ (kc->u.wide[1]));
              (tt0 = tt0 ^ (kc->u.wide[8]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[20]) ^ (kc->u.wide[2]));
              (tt3 = (kc->u.wide[9]) ^ (kc->u.wide[11]));
              (tt0 = tt0 ^ (kc->u.wide[18]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[5]) ^ (kc->u.wide[12]));
              (tt1 = (kc->u.wide[19]) ^ (kc->u.wide[21]));
              (tt0 = tt0 ^ (kc->u.wide[3]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[15]) ^ (kc->u.wide[22]));
              (tt3 = (kc->u.wide[4]) ^ (kc->u.wide[6]));
              (tt0 = tt0 ^ (kc->u.wide[13]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[7]));
              (tt1 = (kc->u.wide[14]) ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ (kc->u.wide[23]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[10]) ^ (kc->u.wide[17]));
              (tt3 = (kc->u.wide[24]) ^ (kc->u.wide[1]));
              (tt0 = tt0 ^ (kc->u.wide[8]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t0);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t0);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t0);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t0);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t1);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t1);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t1);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t1);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t1);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t2);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t2);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t2);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t2);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t2);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t3);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t3);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t3);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t3);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t3);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t4);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t4);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t4);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t4);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (36)) |
                                 (((kc->u.wide[7])) >> (64 - (36)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (3)) |
                                  (((kc->u.wide[14])) >> (64 - (3)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (41)) |
                                  (((kc->u.wide[16])) >> (64 - (41)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (18)) |
                                  (((kc->u.wide[23])) >> (64 - (18)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (1)) |
                                  (((kc->u.wide[20])) >> (64 - (1)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (44)) |
                                 (((kc->u.wide[2])) >> (64 - (44)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (10)) |
                                 (((kc->u.wide[9])) >> (64 - (10)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (45)) |
                                  (((kc->u.wide[11])) >> (64 - (45)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (2)) |
                                  (((kc->u.wide[18])) >> (64 - (2)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (62)) |
                                  (((kc->u.wide[15])) >> (64 - (62)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (6)) |
                                  (((kc->u.wide[22])) >> (64 - (6)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (43)) |
                                 (((kc->u.wide[4])) >> (64 - (43)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (15)) |
                                 (((kc->u.wide[6])) >> (64 - (15)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (61)) |
                                  (((kc->u.wide[13])) >> (64 - (61)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (28)) |
                                  (((kc->u.wide[10])) >> (64 - (28)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (55)) |
                                  (((kc->u.wide[17])) >> (64 - (55)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (25)) |
                                  (((kc->u.wide[24])) >> (64 - (25)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (21)) |
                                 (((kc->u.wide[1])) >> (64 - (21)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (56)) |
                                 (((kc->u.wide[8])) >> (64 - (56)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (27)) |
                                 (((kc->u.wide[5])) >> (64 - (27)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (20)) |
                                  (((kc->u.wide[12])) >> (64 - (20)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (39)) |
                                  (((kc->u.wide[19])) >> (64 - (39)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (8)) |
                                  (((kc->u.wide[21])) >> (64 - (8)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (14)) |
                                 (((kc->u.wide[3])) >> (64 - (14)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[4])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) | (kc->u.wide[4]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[1]));
              (c1 = (kc->u.wide[2]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[1]) & (kc->u.wide[3]));
              (c2 = (kc->u.wide[4]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[3]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[1]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[2]));
              (c4 = (kc->u.wide[3]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[2]) = c1);
            ((kc->u.wide[4]) = c2);
            ((kc->u.wide[1]) = c3);
            ((kc->u.wide[3]) = c4);
            (bnn = ((~(kc->u.wide[13])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[12]) | (kc->u.wide[14]));
              (c0 = (kc->u.wide[10]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) & (kc->u.wide[11]));
              (c1 = (kc->u.wide[12]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[11]) | bnn);
              (c2 = (kc->u.wide[14]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[13]) | (kc->u.wide[10]));
              (c3 = (kc->u.wide[11]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[10]) & (kc->u.wide[12]));
              (c4 = (kc->u.wide[13]) ^ kt);
            } while (0);
            ((kc->u.wide[10]) = c0);
            ((kc->u.wide[12]) = c1);
            ((kc->u.wide[14]) = c2);
            ((kc->u.wide[11]) = c3);
            ((kc->u.wide[13]) = c4);
            (bnn = ((~(kc->u.wide[21])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[22]) | (kc->u.wide[24]));
              (c0 = (kc->u.wide[20]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[24]) & (kc->u.wide[21]));
              (c1 = (kc->u.wide[22]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[23]));
              (c2 = (kc->u.wide[24]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[23]) | (kc->u.wide[20]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) & (kc->u.wide[22]));
              (c4 = (kc->u.wide[23]) ^ kt);
            } while (0);
            ((kc->u.wide[20]) = c0);
            ((kc->u.wide[22]) = c1);
            ((kc->u.wide[24]) = c2);
            ((kc->u.wide[21]) = c3);
            ((kc->u.wide[23]) = c4);
            (bnn = ((~(kc->u.wide[6])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[7]) & (kc->u.wide[9]));
              (c0 = (kc->u.wide[5]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[9]) | (kc->u.wide[6]));
              (c1 = (kc->u.wide[7]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[8]));
              (c2 = (kc->u.wide[9]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[8]) & (kc->u.wide[5]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[5]) | (kc->u.wide[7]));
              (c4 = (kc->u.wide[8]) ^ kt);
            } while (0);
            ((kc->u.wide[5]) = c0);
            ((kc->u.wide[7]) = c1);
            ((kc->u.wide[9]) = c2);
            ((kc->u.wide[6]) = c3);
            ((kc->u.wide[8]) = c4);
            (bnn = ((~(kc->u.wide[17])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[19]));
              (c0 = (kc->u.wide[15]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[19]) | (kc->u.wide[16]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[16]) & (kc->u.wide[18]));
              (c2 = (kc->u.wide[19]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[18]) | (kc->u.wide[15]));
              (c3 = (kc->u.wide[16]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[15]) & (kc->u.wide[17]));
              (c4 = (kc->u.wide[18]) ^ kt);
            } while (0);
            ((kc->u.wide[15]) = c0);
            ((kc->u.wide[17]) = c1);
            ((kc->u.wide[19]) = c2);
            ((kc->u.wide[16]) = c3);
            ((kc->u.wide[18]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 5]);
        } while (0);
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[2]) ^ (kc->u.wide[12]));
              (tt1 = (kc->u.wide[22]) ^ (kc->u.wide[7]));
              (tt0 = tt0 ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[3]) ^ (kc->u.wide[13]));
              (tt3 = (kc->u.wide[23]) ^ (kc->u.wide[8]));
              (tt0 = tt0 ^ (kc->u.wide[18]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[4]) ^ (kc->u.wide[14]));
              (tt1 = (kc->u.wide[24]) ^ (kc->u.wide[9]));
              (tt0 = tt0 ^ (kc->u.wide[19]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[10]));
              (tt3 = (kc->u.wide[20]) ^ (kc->u.wide[5]));
              (tt0 = tt0 ^ (kc->u.wide[15]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[1]) ^ (kc->u.wide[11]));
              (tt1 = (kc->u.wide[21]) ^ (kc->u.wide[6]));
              (tt0 = tt0 ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[2]) ^ (kc->u.wide[12]));
              (tt3 = (kc->u.wide[22]) ^ (kc->u.wide[7]));
              (tt0 = tt0 ^ (kc->u.wide[17]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[3]) ^ (kc->u.wide[13]));
              (tt1 = (kc->u.wide[23]) ^ (kc->u.wide[8]));
              (tt0 = tt0 ^ (kc->u.wide[18]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[4]) ^ (kc->u.wide[14]));
              (tt3 = (kc->u.wide[24]) ^ (kc->u.wide[9]));
              (tt0 = tt0 ^ (kc->u.wide[19]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[10]));
              (tt1 = (kc->u.wide[20]) ^ (kc->u.wide[5]));
              (tt0 = tt0 ^ (kc->u.wide[15]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[1]) ^ (kc->u.wide[11]));
              (tt3 = (kc->u.wide[21]) ^ (kc->u.wide[6]));
              (tt0 = tt0 ^ (kc->u.wide[16]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t0);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t0);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t0);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t0);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t1);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t1);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t1);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t1);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t1);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t2);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t2);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t2);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t2);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t2);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t3);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t3);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t3);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t3);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t3);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t4);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t4);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t4);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t4);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (36)) |
                                  (((kc->u.wide[10])) >> (64 - (36)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (3)) |
                                  (((kc->u.wide[20])) >> (64 - (3)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (41)) |
                                 (((kc->u.wide[5])) >> (64 - (41)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (18)) |
                                  (((kc->u.wide[15])) >> (64 - (18)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (1)) |
                                 (((kc->u.wide[2])) >> (64 - (1)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (44)) |
                                  (((kc->u.wide[12])) >> (64 - (44)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (10)) |
                                  (((kc->u.wide[22])) >> (64 - (10)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (45)) |
                                 (((kc->u.wide[7])) >> (64 - (45)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (2)) |
                                  (((kc->u.wide[17])) >> (64 - (2)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (62)) |
                                 (((kc->u.wide[4])) >> (64 - (62)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (6)) |
                                  (((kc->u.wide[14])) >> (64 - (6)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (43)) |
                                  (((kc->u.wide[24])) >> (64 - (43)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (15)) |
                                 (((kc->u.wide[9])) >> (64 - (15)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (61)) |
                                  (((kc->u.wide[19])) >> (64 - (61)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (28)) |
                                 (((kc->u.wide[1])) >> (64 - (28)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (55)) |
                                  (((kc->u.wide[11])) >> (64 - (55)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (25)) |
                                  (((kc->u.wide[21])) >> (64 - (25)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (21)) |
                                 (((kc->u.wide[6])) >> (64 - (21)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (56)) |
                                  (((kc->u.wide[16])) >> (64 - (56)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (27)) |
                                 (((kc->u.wide[3])) >> (64 - (27)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (20)) |
                                  (((kc->u.wide[13])) >> (64 - (20)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (39)) |
                                  (((kc->u.wide[23])) >> (64 - (39)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (8)) |
                                 (((kc->u.wide[8])) >> (64 - (8)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (14)) |
                                  (((kc->u.wide[18])) >> (64 - (14)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[24])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[12]) | (kc->u.wide[24]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[6]));
              (c1 = (kc->u.wide[12]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[6]) & (kc->u.wide[18]));
              (c2 = (kc->u.wide[24]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[18]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[6]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[12]));
              (c4 = (kc->u.wide[18]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[12]) = c1);
            ((kc->u.wide[24]) = c2);
            ((kc->u.wide[6]) = c3);
            ((kc->u.wide[18]) = c4);
            (bnn = ((~(kc->u.wide[19])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[13]) | (kc->u.wide[20]));
              (c0 = (kc->u.wide[1]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) & (kc->u.wide[7]));
              (c1 = (kc->u.wide[13]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[7]) | bnn);
              (c2 = (kc->u.wide[20]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[19]) | (kc->u.wide[1]));
              (c3 = (kc->u.wide[7]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[1]) & (kc->u.wide[13]));
              (c4 = (kc->u.wide[19]) ^ kt);
            } while (0);
            ((kc->u.wide[1]) = c0);
            ((kc->u.wide[13]) = c1);
            ((kc->u.wide[20]) = c2);
            ((kc->u.wide[7]) = c3);
            ((kc->u.wide[19]) = c4);
            (bnn = ((~(kc->u.wide[8])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) | (kc->u.wide[21]));
              (c0 = (kc->u.wide[2]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[21]) & (kc->u.wide[8]));
              (c1 = (kc->u.wide[14]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[15]));
              (c2 = (kc->u.wide[21]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[15]) | (kc->u.wide[2]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) & (kc->u.wide[14]));
              (c4 = (kc->u.wide[15]) ^ kt);
            } while (0);
            ((kc->u.wide[2]) = c0);
            ((kc->u.wide[14]) = c1);
            ((kc->u.wide[21]) = c2);
            ((kc->u.wide[8]) = c3);
            ((kc->u.wide[15]) = c4);
            (bnn = ((~(kc->u.wide[9])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[10]) & (kc->u.wide[22]));
              (c0 = (kc->u.wide[3]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[22]) | (kc->u.wide[9]));
              (c1 = (kc->u.wide[10]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[16]));
              (c2 = (kc->u.wide[22]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[16]) & (kc->u.wide[3]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[3]) | (kc->u.wide[10]));
              (c4 = (kc->u.wide[16]) ^ kt);
            } while (0);
            ((kc->u.wide[3]) = c0);
            ((kc->u.wide[10]) = c1);
            ((kc->u.wide[22]) = c2);
            ((kc->u.wide[9]) = c3);
            ((kc->u.wide[16]) = c4);
            (bnn = ((~(kc->u.wide[11])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[23]));
              (c0 = (kc->u.wide[4]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[23]) | (kc->u.wide[5]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[5]) & (kc->u.wide[17]));
              (c2 = (kc->u.wide[23]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[17]) | (kc->u.wide[4]));
              (c3 = (kc->u.wide[5]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[4]) & (kc->u.wide[11]));
              (c4 = (kc->u.wide[17]) ^ kt);
            } while (0);
            ((kc->u.wide[4]) = c0);
            ((kc->u.wide[11]) = c1);
            ((kc->u.wide[23]) = c2);
            ((kc->u.wide[5]) = c3);
            ((kc->u.wide[17]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 6]);
        } while (0);
        do {
          do {
            sph_u64 t0;
            sph_u64 t1;
            sph_u64 t2;
            sph_u64 t3;
            sph_u64 t4;
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[12]) ^ (kc->u.wide[13]));
              (tt1 = (kc->u.wide[14]) ^ (kc->u.wide[10]));
              (tt0 = tt0 ^ (kc->u.wide[11]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[18]) ^ (kc->u.wide[19]));
              (tt3 = (kc->u.wide[15]) ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ (kc->u.wide[17]));
              (tt2 = tt2 ^ tt3);
              (t0 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[24]) ^ (kc->u.wide[20]));
              (tt1 = (kc->u.wide[21]) ^ (kc->u.wide[22]));
              (tt0 = tt0 ^ (kc->u.wide[23]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[0]) ^ (kc->u.wide[1]));
              (tt3 = (kc->u.wide[2]) ^ (kc->u.wide[3]));
              (tt0 = tt0 ^ (kc->u.wide[4]));
              (tt2 = tt2 ^ tt3);
              (t1 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[6]) ^ (kc->u.wide[7]));
              (tt1 = (kc->u.wide[8]) ^ (kc->u.wide[9]));
              (tt0 = tt0 ^ (kc->u.wide[5]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[12]) ^ (kc->u.wide[13]));
              (tt3 = (kc->u.wide[14]) ^ (kc->u.wide[10]));
              (tt0 = tt0 ^ (kc->u.wide[11]));
              (tt2 = tt2 ^ tt3);
              (t2 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[18]) ^ (kc->u.wide[19]));
              (tt1 = (kc->u.wide[15]) ^ (kc->u.wide[16]));
              (tt0 = tt0 ^ (kc->u.wide[17]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[24]) ^ (kc->u.wide[20]));
              (tt3 = (kc->u.wide[21]) ^ (kc->u.wide[22]));
              (tt0 = tt0 ^ (kc->u.wide[23]));
              (tt2 = tt2 ^ tt3);
              (t3 = tt0 ^ tt2);
            } while (0);
            do {
              sph_u64 tt0;
              sph_u64 tt1;
              sph_u64 tt2;
              sph_u64 tt3;
              (tt0 = (kc->u.wide[0]) ^ (kc->u.wide[1]));
              (tt1 = (kc->u.wide[2]) ^ (kc->u.wide[3]));
              (tt0 = tt0 ^ (kc->u.wide[4]));
              (tt0 = tt0 ^ tt1);
              (tt0 = ((((tt0) << (1)) | ((tt0) >> (64 - (1)))) &
                      ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
              (tt2 = (kc->u.wide[6]) ^ (kc->u.wide[7]));
              (tt3 = (kc->u.wide[8]) ^ (kc->u.wide[9]));
              (tt0 = tt0 ^ (kc->u.wide[5]));
              (tt2 = tt2 ^ tt3);
              (t4 = tt0 ^ tt2);
            } while (0);
            ((kc->u.wide[0]) = (kc->u.wide[0]) ^ t0);
            ((kc->u.wide[1]) = (kc->u.wide[1]) ^ t0);
            ((kc->u.wide[2]) = (kc->u.wide[2]) ^ t0);
            ((kc->u.wide[3]) = (kc->u.wide[3]) ^ t0);
            ((kc->u.wide[4]) = (kc->u.wide[4]) ^ t0);
            ((kc->u.wide[12]) = (kc->u.wide[12]) ^ t1);
            ((kc->u.wide[13]) = (kc->u.wide[13]) ^ t1);
            ((kc->u.wide[14]) = (kc->u.wide[14]) ^ t1);
            ((kc->u.wide[10]) = (kc->u.wide[10]) ^ t1);
            ((kc->u.wide[11]) = (kc->u.wide[11]) ^ t1);
            ((kc->u.wide[24]) = (kc->u.wide[24]) ^ t2);
            ((kc->u.wide[20]) = (kc->u.wide[20]) ^ t2);
            ((kc->u.wide[21]) = (kc->u.wide[21]) ^ t2);
            ((kc->u.wide[22]) = (kc->u.wide[22]) ^ t2);
            ((kc->u.wide[23]) = (kc->u.wide[23]) ^ t2);
            ((kc->u.wide[6]) = (kc->u.wide[6]) ^ t3);
            ((kc->u.wide[7]) = (kc->u.wide[7]) ^ t3);
            ((kc->u.wide[8]) = (kc->u.wide[8]) ^ t3);
            ((kc->u.wide[9]) = (kc->u.wide[9]) ^ t3);
            ((kc->u.wide[5]) = (kc->u.wide[5]) ^ t3);
            ((kc->u.wide[18]) = (kc->u.wide[18]) ^ t4);
            ((kc->u.wide[19]) = (kc->u.wide[19]) ^ t4);
            ((kc->u.wide[15]) = (kc->u.wide[15]) ^ t4);
            ((kc->u.wide[16]) = (kc->u.wide[16]) ^ t4);
            ((kc->u.wide[17]) = (kc->u.wide[17]) ^ t4);
          } while (0);
          do {
            ((kc->u.wide[1]) = (((((kc->u.wide[1])) << (36)) |
                                 (((kc->u.wide[1])) >> (64 - (36)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[2]) = (((((kc->u.wide[2])) << (3)) |
                                 (((kc->u.wide[2])) >> (64 - (3)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[3]) = (((((kc->u.wide[3])) << (41)) |
                                 (((kc->u.wide[3])) >> (64 - (41)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[4]) = (((((kc->u.wide[4])) << (18)) |
                                 (((kc->u.wide[4])) >> (64 - (18)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[12]) = (((((kc->u.wide[12])) << (1)) |
                                  (((kc->u.wide[12])) >> (64 - (1)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[13]) = (((((kc->u.wide[13])) << (44)) |
                                  (((kc->u.wide[13])) >> (64 - (44)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[14]) = (((((kc->u.wide[14])) << (10)) |
                                  (((kc->u.wide[14])) >> (64 - (10)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[10]) = (((((kc->u.wide[10])) << (45)) |
                                  (((kc->u.wide[10])) >> (64 - (45)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[11]) = (((((kc->u.wide[11])) << (2)) |
                                  (((kc->u.wide[11])) >> (64 - (2)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[24]) = (((((kc->u.wide[24])) << (62)) |
                                  (((kc->u.wide[24])) >> (64 - (62)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[20]) = (((((kc->u.wide[20])) << (6)) |
                                  (((kc->u.wide[20])) >> (64 - (6)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[21]) = (((((kc->u.wide[21])) << (43)) |
                                  (((kc->u.wide[21])) >> (64 - (43)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[22]) = (((((kc->u.wide[22])) << (15)) |
                                  (((kc->u.wide[22])) >> (64 - (15)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[23]) = (((((kc->u.wide[23])) << (61)) |
                                  (((kc->u.wide[23])) >> (64 - (61)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[6]) = (((((kc->u.wide[6])) << (28)) |
                                 (((kc->u.wide[6])) >> (64 - (28)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[7]) = (((((kc->u.wide[7])) << (55)) |
                                 (((kc->u.wide[7])) >> (64 - (55)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[8]) = (((((kc->u.wide[8])) << (25)) |
                                 (((kc->u.wide[8])) >> (64 - (25)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[9]) = (((((kc->u.wide[9])) << (21)) |
                                 (((kc->u.wide[9])) >> (64 - (21)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[5]) = (((((kc->u.wide[5])) << (56)) |
                                 (((kc->u.wide[5])) >> (64 - (56)))) &
                                ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[18]) = (((((kc->u.wide[18])) << (27)) |
                                  (((kc->u.wide[18])) >> (64 - (27)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[19]) = (((((kc->u.wide[19])) << (20)) |
                                  (((kc->u.wide[19])) >> (64 - (20)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[15]) = (((((kc->u.wide[15])) << (39)) |
                                  (((kc->u.wide[15])) >> (64 - (39)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[16]) = (((((kc->u.wide[16])) << (8)) |
                                  (((kc->u.wide[16])) >> (64 - (8)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            ((kc->u.wide[17]) = (((((kc->u.wide[17])) << (14)) |
                                  (((kc->u.wide[17])) >> (64 - (14)))) &
                                 ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
          } while (0);
          do {
            sph_u64 c0;
            sph_u64 c1;
            sph_u64 c2;
            sph_u64 c3;
            sph_u64 c4;
            sph_u64 bnn;
            (bnn = ((~(kc->u.wide[21])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[13]) | (kc->u.wide[21]));
              (c0 = (kc->u.wide[0]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[9]));
              (c1 = (kc->u.wide[13]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[9]) & (kc->u.wide[17]));
              (c2 = (kc->u.wide[21]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[17]) | (kc->u.wide[0]));
              (c3 = (kc->u.wide[9]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[0]) & (kc->u.wide[13]));
              (c4 = (kc->u.wide[17]) ^ kt);
            } while (0);
            ((kc->u.wide[0]) = c0);
            ((kc->u.wide[13]) = c1);
            ((kc->u.wide[21]) = c2);
            ((kc->u.wide[9]) = c3);
            ((kc->u.wide[17]) = c4);
            (bnn = ((~(kc->u.wide[23])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[19]) | (kc->u.wide[2]));
              (c0 = (kc->u.wide[6]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[2]) & (kc->u.wide[10]));
              (c1 = (kc->u.wide[19]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[10]) | bnn);
              (c2 = (kc->u.wide[2]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[23]) | (kc->u.wide[6]));
              (c3 = (kc->u.wide[10]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[6]) & (kc->u.wide[19]));
              (c4 = (kc->u.wide[23]) ^ kt);
            } while (0);
            ((kc->u.wide[6]) = c0);
            ((kc->u.wide[19]) = c1);
            ((kc->u.wide[2]) = c2);
            ((kc->u.wide[10]) = c3);
            ((kc->u.wide[23]) = c4);
            (bnn = ((~(kc->u.wide[16])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[20]) | (kc->u.wide[8]));
              (c0 = (kc->u.wide[12]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[8]) & (kc->u.wide[16]));
              (c1 = (kc->u.wide[20]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[4]));
              (c2 = (kc->u.wide[8]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[4]) | (kc->u.wide[12]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[12]) & (kc->u.wide[20]));
              (c4 = (kc->u.wide[4]) ^ kt);
            } while (0);
            ((kc->u.wide[12]) = c0);
            ((kc->u.wide[20]) = c1);
            ((kc->u.wide[8]) = c2);
            ((kc->u.wide[16]) = c3);
            ((kc->u.wide[4]) = c4);
            (bnn = ((~(kc->u.wide[22])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[1]) & (kc->u.wide[14]));
              (c0 = (kc->u.wide[18]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[14]) | (kc->u.wide[22]));
              (c1 = (kc->u.wide[1]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = bnn | (kc->u.wide[5]));
              (c2 = (kc->u.wide[14]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[5]) & (kc->u.wide[18]));
              (c3 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[18]) | (kc->u.wide[1]));
              (c4 = (kc->u.wide[5]) ^ kt);
            } while (0);
            ((kc->u.wide[18]) = c0);
            ((kc->u.wide[1]) = c1);
            ((kc->u.wide[14]) = c2);
            ((kc->u.wide[22]) = c3);
            ((kc->u.wide[5]) = c4);
            (bnn = ((~(kc->u.wide[7])) & ((sph_u64)(0xFFFFFFFFFFFFFFFFUL))));
            do {
              sph_u64 kt;
              (kt = bnn & (kc->u.wide[15]));
              (c0 = (kc->u.wide[24]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[15]) | (kc->u.wide[3]));
              (c1 = bnn ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[3]) & (kc->u.wide[11]));
              (c2 = (kc->u.wide[15]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[11]) | (kc->u.wide[24]));
              (c3 = (kc->u.wide[3]) ^ kt);
            } while (0);
            do {
              sph_u64 kt;
              (kt = (kc->u.wide[24]) & (kc->u.wide[7]));
              (c4 = (kc->u.wide[11]) ^ kt);
            } while (0);
            ((kc->u.wide[24]) = c0);
            ((kc->u.wide[7]) = c1);
            ((kc->u.wide[15]) = c2);
            ((kc->u.wide[3]) = c3);
            ((kc->u.wide[11]) = c4);
          } while (0);
          ((kc->u.wide[0]) = (kc->u.wide[0]) ^ RC[j + 7]);
        } while (0);
        do {
          sph_u64 t;
          (t = (kc->u.wide[5]));
          ((kc->u.wide[5]) = (kc->u.wide[6]));
          ((kc->u.wide[6]) = (kc->u.wide[19]));
          ((kc->u.wide[19]) = t);
          (t = (kc->u.wide[10]));
          ((kc->u.wide[10]) = (kc->u.wide[12]));
          ((kc->u.wide[12]) = (kc->u.wide[8]));
          ((kc->u.wide[8]) = t);
          (t = (kc->u.wide[15]));
          ((kc->u.wide[15]) = (kc->u.wide[18]));
          ((kc->u.wide[18]) = (kc->u.wide[22]));
          ((kc->u.wide[22]) = t);
          (t = (kc->u.wide[20]));
          ((kc->u.wide[20]) = (kc->u.wide[24]));
          ((kc->u.wide[24]) = (kc->u.wide[11]));
          ((kc->u.wide[11]) = t);
          (t = (kc->u.wide[1]));
          ((kc->u.wide[1]) = (kc->u.wide[13]));
          ((kc->u.wide[13]) = (kc->u.wide[16]));
          ((kc->u.wide[16]) = t);
          (t = (kc->u.wide[21]));
          ((kc->u.wide[21]) = (kc->u.wide[7]));
          ((kc->u.wide[7]) = (kc->u.wide[2]));
          ((kc->u.wide[2]) = t);
          (t = (kc->u.wide[17]));
          ((kc->u.wide[17]) = (kc->u.wide[14]));
          ((kc->u.wide[14]) = (kc->u.wide[4]));
          ((kc->u.wide[4]) = t);
          (t = (kc->u.wide[3]));
          ((kc->u.wide[3]) = (kc->u.wide[9]));
          ((kc->u.wide[9]) = (kc->u.wide[23]));
          ((kc->u.wide[23]) = t);
        } while (0);
      }
    } while (0);
                        ptr = 0;
                }
        }
        keccak_WRITE_STATE(kc);
        kc->ptr = ptr;
}


#define DEFCLOSE(d, lim) \
        static void keccak_close ## d( \
                sph_keccak_context *kc, unsigned ub, unsigned n, void *dst) \
        { \
                unsigned eb; \
                union { \
                        unsigned char tmp[lim + 1]; \
                        sph_u64 dummy;   /* for alignment */ \
                } u; \
                size_t j; \
 \
                eb = (0x100 | (ub & 0xFF)) >> (8 - n); \
                if (kc->ptr == (lim - 1)) { \
                        if (n == 7) { \
                                u.tmp[0] = eb; \
                                memset(u.tmp + 1, 0, lim - 1); \
                                u.tmp[lim] = 0x80; \
                                j = 1 + lim; \
                        } else { \
                                u.tmp[0] = eb | 0x80; \
                                j = 1; \
                        } \
                } else { \
                        j = lim - kc->ptr; \
                        u.tmp[0] = eb; \
                        memset(u.tmp + 1, 0, j - 2); \
                        u.tmp[j - 1] = 0x80; \
                } \
                keccak_core(kc, u.tmp, j, lim); \
                /* Finalize the "lane complement" */ \
                kc->u.wide[ 1] = ~kc->u.wide[ 1]; \
                kc->u.wide[ 2] = ~kc->u.wide[ 2]; \
                kc->u.wide[ 8] = ~kc->u.wide[ 8]; \
                kc->u.wide[12] = ~kc->u.wide[12]; \
                kc->u.wide[17] = ~kc->u.wide[17]; \
                kc->u.wide[20] = ~kc->u.wide[20]; \
                for (j = 0; j < d; j += 8) \
                        sph_enc64le_aligned(u.tmp + j, kc->u.wide[j >> 3]); \
                memcpy(dst, u.tmp, d); \
                keccak_init(kc, (unsigned)d << 3); \
        } \


DEFCLOSE(28, 144)
DEFCLOSE(32, 136)
DEFCLOSE(48, 104)
DEFCLOSE(64, 72)

void
sph_keccak224_init(sph_keccak_context *cc)
{
        keccak_init(cc, 224);
}

void
sph_keccak224(sph_keccak_context *cc, const unsigned char *data, size_t len)
{
        keccak_core(cc, data, len, 144);
}

void
sph_keccak224_close(sph_keccak_context *cc, unsigned char *dst)
{
        sph_keccak224_addbits_and_close(cc, 0, 0, dst);
}

void
sph_keccak224_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        keccak_close28(cc, ub, n, dst);
}

void
sph_keccak256_init(sph_keccak_context *cc)
{
        keccak_init(cc, 256);
}

void
sph_keccak256(sph_keccak_context *cc, const unsigned char *data, size_t len)
{
        keccak_core(cc, data, len, 136);
}

void
sph_keccak256_close(sph_keccak_context *cc, unsigned char *dst)
{
        sph_keccak256_addbits_and_close(cc, 0, 0, dst);
}

void
sph_keccak256_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        keccak_close32(cc, ub, n, dst);
}

void
sph_keccak384_init(sph_keccak_context *cc)
{
        keccak_init(cc, 384);
}

void
sph_keccak384(sph_keccak_context *cc, const unsigned char *data, size_t len)
{
        keccak_core(cc, data, len, 104);
}

void
sph_keccak384_close(sph_keccak_context *cc, unsigned char *dst)
{
        sph_keccak384_addbits_and_close(cc, 0, 0, dst);
}

void
sph_keccak384_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        keccak_close48(cc, ub, n, dst);
}

void
sph_keccak512_init(sph_keccak_context *cc)
{
        keccak_init(cc, 512);
}

void
sph_keccak512(sph_keccak_context *cc, const unsigned char *data, size_t len)
{
        keccak_core(cc, data, len, 72);
}

void
sph_keccak512_close(sph_keccak_context *cc, unsigned char *dst)
{
        sph_keccak512_addbits_and_close(cc, 0, 0, dst);
}

void
sph_keccak512_addbits_and_close(sph_keccak_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        keccak_close64(cc, ub, n, dst);
}

















#define SPH_SIZE_luffa224   224
#define SPH_SIZE_luffa256   256
#define SPH_SIZE_luffa384   384
#define SPH_SIZE_luffa512   512

typedef struct {
        unsigned char buf[32];    /* first field, for alignment */
        size_t ptr;
        sph_u32 V[3][8];
} sph_luffa224_context;

typedef sph_luffa224_context sph_luffa256_context;

typedef struct {
        unsigned char buf[32];    /* first field, for alignment */
        size_t ptr;
        sph_u32 V[4][8];
} sph_luffa384_context;

typedef struct {
        unsigned char buf[32];    /* first field, for alignment */
        size_t ptr;
        sph_u32 V[5][8];
} sph_luffa512_context;

void
sph_luffa224_init(sph_luffa224_context *cc);
void
sph_luffa224(sph_luffa224_context *cc, const unsigned char *data, size_t len);
void
sph_luffa224_close(sph_luffa224_context *cc, unsigned char *dst);
void
sph_luffa224_addbits_and_close(sph_luffa224_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_luffa256_init(sph_luffa256_context *cc);
void
sph_luffa256(sph_luffa256_context *cc, const unsigned char *data, size_t len);
void
sph_luffa256_close(sph_luffa256_context *cc, unsigned char *dst);
void
sph_luffa256_addbits_and_close(sph_luffa256_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_luffa384_init(sph_luffa384_context *cc);
void
sph_luffa384(sph_luffa384_context *cc, const unsigned char *data, size_t len);
void
sph_luffa384_close(sph_luffa384_context *cc, unsigned char *dst);
void
sph_luffa384_addbits_and_close(sph_luffa384_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_luffa512_init(sph_luffa512_context *cc);
void
sph_luffa512(sph_luffa512_context *cc, const unsigned char *data, size_t len);
void
sph_luffa512_close(sph_luffa512_context *cc, unsigned char *dst);
void
sph_luffa512_addbits_and_close(sph_luffa512_context *cc, unsigned ub, unsigned n, unsigned char *dst);






#define SPH_LUFFA_PARALLEL   1


static const sph_u32 V_INIT[5][8] = {
        {
                SPH_C32(0x6d251e69), SPH_C32(0x44b051e0),
                SPH_C32(0x4eaa6fb4), SPH_C32(0xdbf78465),
                SPH_C32(0x6e292011), SPH_C32(0x90152df4),
                SPH_C32(0xee058139), SPH_C32(0xdef610bb)
        }, {
                SPH_C32(0xc3b44b95), SPH_C32(0xd9d2f256),
                SPH_C32(0x70eee9a0), SPH_C32(0xde099fa3),
                SPH_C32(0x5d9b0557), SPH_C32(0x8fc944b3),
                SPH_C32(0xcf1ccf0e), SPH_C32(0x746cd581)
        }, {
                SPH_C32(0xf7efc89d), SPH_C32(0x5dba5781),
                SPH_C32(0x04016ce5), SPH_C32(0xad659c05),
                SPH_C32(0x0306194f), SPH_C32(0x666d1836),
                SPH_C32(0x24aa230a), SPH_C32(0x8b264ae7)
        }, {
                SPH_C32(0x858075d5), SPH_C32(0x36d79cce),
                SPH_C32(0xe571f7d7), SPH_C32(0x204b1f67),
                SPH_C32(0x35870c6a), SPH_C32(0x57e9e923),
                SPH_C32(0x14bcb808), SPH_C32(0x7cde72ce)
        }, {
                SPH_C32(0x6c68e9be), SPH_C32(0x5ec41e22),
                SPH_C32(0xc825b7c7), SPH_C32(0xaffb4363),
                SPH_C32(0xf5df3999), SPH_C32(0x0fc688f1),
                SPH_C32(0xb07224cc), SPH_C32(0x03e86cea)
        }
};

/* 32 bit */
static const sph_u32 RC00[8] = {
        SPH_C32(0x303994a6), SPH_C32(0xc0e65299),
        SPH_C32(0x6cc33a12), SPH_C32(0xdc56983e),
        SPH_C32(0x1e00108f), SPH_C32(0x7800423d),
        SPH_C32(0x8f5b7882), SPH_C32(0x96e1db12)
};

/* 32 bit */
static const sph_u32 RC04[8] = {
        SPH_C32(0xe0337818), SPH_C32(0x441ba90d),
        SPH_C32(0x7f34d442), SPH_C32(0x9389217f),
        SPH_C32(0xe5a8bce6), SPH_C32(0x5274baf4),
        SPH_C32(0x26889ba7), SPH_C32(0x9a226e9d)
};

/* 32 bit */
static const sph_u32 RC10[8] = {
        SPH_C32(0xb6de10ed), SPH_C32(0x70f47aae),
        SPH_C32(0x0707a3d4), SPH_C32(0x1c1e8f51),
        SPH_C32(0x707a3d45), SPH_C32(0xaeb28562),
        SPH_C32(0xbaca1589), SPH_C32(0x40a46f3e)
};

/* 32 bit */
static const sph_u32 RC14[8] = {
        SPH_C32(0x01685f3d), SPH_C32(0x05a17cf4),
        SPH_C32(0xbd09caca), SPH_C32(0xf4272b28),
        SPH_C32(0x144ae5cc), SPH_C32(0xfaa7ae2b),
        SPH_C32(0x2e48f1c1), SPH_C32(0xb923c704)
};


static const sph_u64 RCW010[8] = {
        SPH_C64(0xb6de10ed303994a6), SPH_C64(0x70f47aaec0e65299),
        SPH_C64(0x0707a3d46cc33a12), SPH_C64(0x1c1e8f51dc56983e),
        SPH_C64(0x707a3d451e00108f), SPH_C64(0xaeb285627800423d),
        SPH_C64(0xbaca15898f5b7882), SPH_C64(0x40a46f3e96e1db12)
};

static const sph_u64 RCW014[8] = {
        SPH_C64(0x01685f3de0337818), SPH_C64(0x05a17cf4441ba90d),
        SPH_C64(0xbd09caca7f34d442), SPH_C64(0xf4272b289389217f),
        SPH_C64(0x144ae5cce5a8bce6), SPH_C64(0xfaa7ae2b5274baf4),
        SPH_C64(0x2e48f1c126889ba7), SPH_C64(0xb923c7049a226e9d)
};


static const sph_u32 RC20[8] = {
        SPH_C32(0xfc20d9d2), SPH_C32(0x34552e25),
        SPH_C32(0x7ad8818f), SPH_C32(0x8438764a),
        SPH_C32(0xbb6de032), SPH_C32(0xedb780c8),
        SPH_C32(0xd9847356), SPH_C32(0xa2c78434)
};

static const sph_u32 RC24[8] = {
        SPH_C32(0xe25e72c1), SPH_C32(0xe623bb72),
        SPH_C32(0x5c58a4a4), SPH_C32(0x1e38e2e7),
        SPH_C32(0x78e38b9d), SPH_C32(0x27586719),
        SPH_C32(0x36eda57f), SPH_C32(0x703aace7)
};

/* 32 bit */
static const sph_u32 RC30[8] = {
        SPH_C32(0xb213afa5), SPH_C32(0xc84ebe95),
        SPH_C32(0x4e608a22), SPH_C32(0x56d858fe),
        SPH_C32(0x343b138f), SPH_C32(0xd0ec4e3d),
        SPH_C32(0x2ceb4882), SPH_C32(0xb3ad2208)
};

/* 32 bit */
static const sph_u32 RC34[8] = {
        SPH_C32(0xe028c9bf), SPH_C32(0x44756f91),
        SPH_C32(0x7e8fce32), SPH_C32(0x956548be),
        SPH_C32(0xfe191be2), SPH_C32(0x3cb226e5),
        SPH_C32(0x5944a28e), SPH_C32(0xa1c4c355)
};


static const sph_u64 RCW230[8] = {
        SPH_C64(0xb213afa5fc20d9d2), SPH_C64(0xc84ebe9534552e25),
        SPH_C64(0x4e608a227ad8818f), SPH_C64(0x56d858fe8438764a),
        SPH_C64(0x343b138fbb6de032), SPH_C64(0xd0ec4e3dedb780c8),
        SPH_C64(0x2ceb4882d9847356), SPH_C64(0xb3ad2208a2c78434)
};


static const sph_u64 RCW234[8] = {
        SPH_C64(0xe028c9bfe25e72c1), SPH_C64(0x44756f91e623bb72),
        SPH_C64(0x7e8fce325c58a4a4), SPH_C64(0x956548be1e38e2e7),
        SPH_C64(0xfe191be278e38b9d), SPH_C64(0x3cb226e527586719),
        SPH_C64(0x5944a28e36eda57f), SPH_C64(0xa1c4c355703aace7)
};


static const sph_u32 RC40[8] = {
        SPH_C32(0xf0d2e9e3), SPH_C32(0xac11d7fa),
        SPH_C32(0x1bcb66f2), SPH_C32(0x6f2d9bc9),
        SPH_C32(0x78602649), SPH_C32(0x8edae952),
        SPH_C32(0x3b6ba548), SPH_C32(0xedae9520)
};

static const sph_u32 RC44[8] = {
        SPH_C32(0x5090d577), SPH_C32(0x2d1925ab),
        SPH_C32(0xb46496ac), SPH_C32(0xd1925ab0),
        SPH_C32(0x29131ab6), SPH_C32(0x0fc053c3),
        SPH_C32(0x3f014f0c), SPH_C32(0xfc053c31)
};

#define DECL_TMP8(w) \
        sph_u32 w ## 0, w ## 1, w ## 2, w ## 3, w ## 4, w ## 5, w ## 6, w ## 7;

#define M2(d, s)   do { \
                sph_u32 tmp = s ## 7; \
                d ## 7 = s ## 6; \
                d ## 6 = s ## 5; \
                d ## 5 = s ## 4; \
                d ## 4 = s ## 3 ^ tmp; \
                d ## 3 = s ## 2 ^ tmp; \
                d ## 2 = s ## 1; \
                d ## 1 = s ## 0 ^ tmp; \
                d ## 0 = tmp; \
        } while (0)

#define XOR(d, s1, s2)   do { \
                d ## 0 = s1 ## 0 ^ s2 ## 0; \
                d ## 1 = s1 ## 1 ^ s2 ## 1; \
                d ## 2 = s1 ## 2 ^ s2 ## 2; \
                d ## 3 = s1 ## 3 ^ s2 ## 3; \
                d ## 4 = s1 ## 4 ^ s2 ## 4; \
                d ## 5 = s1 ## 5 ^ s2 ## 5; \
                d ## 6 = s1 ## 6 ^ s2 ## 6; \
                d ## 7 = s1 ## 7 ^ s2 ## 7; \
        } while (0)

#define SUB_CRUMB_GEN(a0, a1, a2, a3, width)   do { \
                sph_u ## width tmp; \
                tmp = (a0); \
                (a0) |= (a1); \
                (a2) ^= (a3); \
                (a1) = SPH_T ## width(~(a1)); \
                (a0) ^= (a3); \
                (a3) &= tmp; \
                (a1) ^= (a3); \
                (a3) ^= (a2); \
                (a2) &= (a0); \
                (a0) = SPH_T ## width(~(a0)); \
                (a2) ^= (a1); \
                (a1) |= (a3); \
                tmp ^= (a1); \
                (a3) ^= (a2); \
                (a2) &= (a1); \
                (a1) ^= (a0); \
                (a0) = tmp; \
        } while (0)

#define SUB_CRUMB(a0, a1, a2, a3)    SUB_CRUMB_GEN(a0, a1, a2, a3, 32)
#define SUB_CRUMBW(a0, a1, a2, a3)   SUB_CRUMB_GEN(a0, a1, a2, a3, 64)

#define MIX_WORDW(u, v)   do { \
                sph_u32 ul, uh, vl, vh; \
                (v) ^= (u); \
                ul = SPH_T32((sph_u32)(u)); \
                uh = SPH_T32((sph_u32)((u) >> 32)); \
                vl = SPH_T32((sph_u32)(v)); \
                vh = SPH_T32((sph_u32)((v) >> 32)); \
                ul = SPH_ROTL32(ul, 2) ^ vl; \
                vl = SPH_ROTL32(vl, 14) ^ ul; \
                ul = SPH_ROTL32(ul, 10) ^ vl; \
                vl = SPH_ROTL32(vl, 1); \
                uh = SPH_ROTL32(uh, 2) ^ vh; \
                vh = SPH_ROTL32(vh, 14) ^ uh; \
                uh = SPH_ROTL32(uh, 10) ^ vh; \
                vh = SPH_ROTL32(vh, 1); \
                (u) = (sph_u64)ul | ((sph_u64)uh << 32); \
                (v) = (sph_u64)vl | ((sph_u64)vh << 32); \
        } while (0)


#define MIX_WORD(u, v)   do { \
                (v) ^= (u); \
                (u) = SPH_ROTL32((u), 2) ^ (v); \
                (v) = SPH_ROTL32((v), 14) ^ (u); \
                (u) = SPH_ROTL32((u), 10) ^ (v); \
                (v) = SPH_ROTL32((v), 1); \
        } while (0)

#define luffa_DECL_STATE3 \
        sph_u32 V00, V01, V02, V03, V04, V05, V06, V07; \
        sph_u32 V10, V11, V12, V13, V14, V15, V16, V17; \
        sph_u32 V20, V21, V22, V23, V24, V25, V26, V27;

#define luffa_READ_STATE3(state)   do { \
                V00 = (state)->V[0][0]; \
                V01 = (state)->V[0][1]; \
                V02 = (state)->V[0][2]; \
                V03 = (state)->V[0][3]; \
                V04 = (state)->V[0][4]; \
                V05 = (state)->V[0][5]; \
                V06 = (state)->V[0][6]; \
                V07 = (state)->V[0][7]; \
                V10 = (state)->V[1][0]; \
                V11 = (state)->V[1][1]; \
                V12 = (state)->V[1][2]; \
                V13 = (state)->V[1][3]; \
                V14 = (state)->V[1][4]; \
                V15 = (state)->V[1][5]; \
                V16 = (state)->V[1][6]; \
                V17 = (state)->V[1][7]; \
                V20 = (state)->V[2][0]; \
                V21 = (state)->V[2][1]; \
                V22 = (state)->V[2][2]; \
                V23 = (state)->V[2][3]; \
                V24 = (state)->V[2][4]; \
                V25 = (state)->V[2][5]; \
                V26 = (state)->V[2][6]; \
                V27 = (state)->V[2][7]; \
        } while (0)

#define luffa_WRITE_STATE3(state)   do { \
                (state)->V[0][0] = V00; \
                (state)->V[0][1] = V01; \
                (state)->V[0][2] = V02; \
                (state)->V[0][3] = V03; \
                (state)->V[0][4] = V04; \
                (state)->V[0][5] = V05; \
                (state)->V[0][6] = V06; \
                (state)->V[0][7] = V07; \
                (state)->V[1][0] = V10; \
                (state)->V[1][1] = V11; \
                (state)->V[1][2] = V12; \
                (state)->V[1][3] = V13; \
                (state)->V[1][4] = V14; \
                (state)->V[1][5] = V15; \
                (state)->V[1][6] = V16; \
                (state)->V[1][7] = V17; \
                (state)->V[2][0] = V20; \
                (state)->V[2][1] = V21; \
                (state)->V[2][2] = V22; \
                (state)->V[2][3] = V23; \
                (state)->V[2][4] = V24; \
                (state)->V[2][5] = V25; \
                (state)->V[2][6] = V26; \
                (state)->V[2][7] = V27; \
        } while (0)

#define MI3   do { \
                DECL_TMP8(M) \
                DECL_TMP8(a) \
                M0 = sph_dec32be_aligned(buf +  0); \
                M1 = sph_dec32be_aligned(buf +  4); \
                M2 = sph_dec32be_aligned(buf +  8); \
                M3 = sph_dec32be_aligned(buf + 12); \
                M4 = sph_dec32be_aligned(buf + 16); \
                M5 = sph_dec32be_aligned(buf + 20); \
                M6 = sph_dec32be_aligned(buf + 24); \
                M7 = sph_dec32be_aligned(buf + 28); \
                XOR(a, V0, V1); \
                XOR(a, a, V2); \
                M2(a, a); \
                XOR(V0, a, V0); \
                XOR(V0, M, V0); \
                M2(M, M); \
                XOR(V1, a, V1); \
                XOR(V1, M, V1); \
                M2(M, M); \
                XOR(V2, a, V2); \
                XOR(V2, M, V2); \
        } while (0)

#define TWEAK3   do { \
                V14 = SPH_ROTL32(V14, 1); \
                V15 = SPH_ROTL32(V15, 1); \
                V16 = SPH_ROTL32(V16, 1); \
                V17 = SPH_ROTL32(V17, 1); \
                V24 = SPH_ROTL32(V24, 2); \
                V25 = SPH_ROTL32(V25, 2); \
                V26 = SPH_ROTL32(V26, 2); \
                V27 = SPH_ROTL32(V27, 2); \
        } while (0)


#define luffa_P3   do { \
                int r; \
                sph_u64 W0, W1, W2, W3, W4, W5, W6, W7; \
                TWEAK3; \
                W0 = (sph_u64)V00 | ((sph_u64)V10 << 32); \
                W1 = (sph_u64)V01 | ((sph_u64)V11 << 32); \
                W2 = (sph_u64)V02 | ((sph_u64)V12 << 32); \
                W3 = (sph_u64)V03 | ((sph_u64)V13 << 32); \
                W4 = (sph_u64)V04 | ((sph_u64)V14 << 32); \
                W5 = (sph_u64)V05 | ((sph_u64)V15 << 32); \
                W6 = (sph_u64)V06 | ((sph_u64)V16 << 32); \
                W7 = (sph_u64)V07 | ((sph_u64)V17 << 32); \
                for (r = 0; r < 8; r ++) { \
                        SUB_CRUMBW(W0, W1, W2, W3); \
                        SUB_CRUMBW(W5, W6, W7, W4); \
                        MIX_WORDW(W0, W4); \
                        MIX_WORDW(W1, W5); \
                        MIX_WORDW(W2, W6); \
                        MIX_WORDW(W3, W7); \
                        W0 ^= RCW010[r]; \
                        W4 ^= RCW014[r]; \
                } \
                V00 = SPH_T32((sph_u32)W0); \
                V10 = SPH_T32((sph_u32)(W0 >> 32)); \
                V01 = SPH_T32((sph_u32)W1); \
                V11 = SPH_T32((sph_u32)(W1 >> 32)); \
                V02 = SPH_T32((sph_u32)W2); \
                V12 = SPH_T32((sph_u32)(W2 >> 32)); \
                V03 = SPH_T32((sph_u32)W3); \
                V13 = SPH_T32((sph_u32)(W3 >> 32)); \
                V04 = SPH_T32((sph_u32)W4); \
                V14 = SPH_T32((sph_u32)(W4 >> 32)); \
                V05 = SPH_T32((sph_u32)W5); \
                V15 = SPH_T32((sph_u32)(W5 >> 32)); \
                V06 = SPH_T32((sph_u32)W6); \
                V16 = SPH_T32((sph_u32)(W6 >> 32)); \
                V07 = SPH_T32((sph_u32)W7); \
                V17 = SPH_T32((sph_u32)(W7 >> 32)); \
                for (r = 0; r < 8; r ++) { \
                        SUB_CRUMB(V20, V21, V22, V23); \
                        SUB_CRUMB(V25, V26, V27, V24); \
                        MIX_WORD(V20, V24); \
                        MIX_WORD(V21, V25); \
                        MIX_WORD(V22, V26); \
                        MIX_WORD(V23, V27); \
                        V20 ^= RC20[r]; \
                        V24 ^= RC24[r]; \
                } \
        } while (0)


#define luffa_DECL_STATE4 \
        sph_u32 V00, V01, V02, V03, V04, V05, V06, V07; \
        sph_u32 V10, V11, V12, V13, V14, V15, V16, V17; \
        sph_u32 V20, V21, V22, V23, V24, V25, V26, V27; \
        sph_u32 V30, V31, V32, V33, V34, V35, V36, V37;

#define luffa_READ_STATE4(state)   do { \
                V00 = (state)->V[0][0]; \
                V01 = (state)->V[0][1]; \
                V02 = (state)->V[0][2]; \
                V03 = (state)->V[0][3]; \
                V04 = (state)->V[0][4]; \
                V05 = (state)->V[0][5]; \
                V06 = (state)->V[0][6]; \
                V07 = (state)->V[0][7]; \
                V10 = (state)->V[1][0]; \
                V11 = (state)->V[1][1]; \
                V12 = (state)->V[1][2]; \
                V13 = (state)->V[1][3]; \
                V14 = (state)->V[1][4]; \
                V15 = (state)->V[1][5]; \
                V16 = (state)->V[1][6]; \
                V17 = (state)->V[1][7]; \
                V20 = (state)->V[2][0]; \
                V21 = (state)->V[2][1]; \
                V22 = (state)->V[2][2]; \
                V23 = (state)->V[2][3]; \
                V24 = (state)->V[2][4]; \
                V25 = (state)->V[2][5]; \
                V26 = (state)->V[2][6]; \
                V27 = (state)->V[2][7]; \
                V30 = (state)->V[3][0]; \
                V31 = (state)->V[3][1]; \
                V32 = (state)->V[3][2]; \
                V33 = (state)->V[3][3]; \
                V34 = (state)->V[3][4]; \
                V35 = (state)->V[3][5]; \
                V36 = (state)->V[3][6]; \
                V37 = (state)->V[3][7]; \
        } while (0)

#define luffa_WRITE_STATE4(state)   do { \
                (state)->V[0][0] = V00; \
                (state)->V[0][1] = V01; \
                (state)->V[0][2] = V02; \
                (state)->V[0][3] = V03; \
                (state)->V[0][4] = V04; \
                (state)->V[0][5] = V05; \
                (state)->V[0][6] = V06; \
                (state)->V[0][7] = V07; \
                (state)->V[1][0] = V10; \
                (state)->V[1][1] = V11; \
                (state)->V[1][2] = V12; \
                (state)->V[1][3] = V13; \
                (state)->V[1][4] = V14; \
                (state)->V[1][5] = V15; \
                (state)->V[1][6] = V16; \
                (state)->V[1][7] = V17; \
                (state)->V[2][0] = V20; \
                (state)->V[2][1] = V21; \
                (state)->V[2][2] = V22; \
                (state)->V[2][3] = V23; \
                (state)->V[2][4] = V24; \
                (state)->V[2][5] = V25; \
                (state)->V[2][6] = V26; \
                (state)->V[2][7] = V27; \
                (state)->V[3][0] = V30; \
                (state)->V[3][1] = V31; \
                (state)->V[3][2] = V32; \
                (state)->V[3][3] = V33; \
                (state)->V[3][4] = V34; \
                (state)->V[3][5] = V35; \
                (state)->V[3][6] = V36; \
                (state)->V[3][7] = V37; \
        } while (0)

#define MI4   do { \
                DECL_TMP8(M) \
                DECL_TMP8(a) \
                DECL_TMP8(b) \
                M0 = sph_dec32be_aligned(buf +  0); \
                M1 = sph_dec32be_aligned(buf +  4); \
                M2 = sph_dec32be_aligned(buf +  8); \
                M3 = sph_dec32be_aligned(buf + 12); \
                M4 = sph_dec32be_aligned(buf + 16); \
                M5 = sph_dec32be_aligned(buf + 20); \
                M6 = sph_dec32be_aligned(buf + 24); \
                M7 = sph_dec32be_aligned(buf + 28); \
                XOR(a, V0, V1); \
                XOR(b, V2, V3); \
                XOR(a, a, b); \
                M2(a, a); \
                XOR(V0, a, V0); \
                XOR(V1, a, V1); \
                XOR(V2, a, V2); \
                XOR(V3, a, V3); \
                M2(b, V0); \
                XOR(b, b, V3); \
                M2(V3, V3); \
                XOR(V3, V3, V2); \
                M2(V2, V2); \
                XOR(V2, V2, V1); \
                M2(V1, V1); \
                XOR(V1, V1, V0); \
                XOR(V0, b, M); \
                M2(M, M); \
                XOR(V1, V1, M); \
                M2(M, M); \
                XOR(V2, V2, M); \
                M2(M, M); \
                XOR(V3, V3, M); \
        } while (0)

#define TWEAK4   do { \
                V14 = SPH_ROTL32(V14, 1); \
                V15 = SPH_ROTL32(V15, 1); \
                V16 = SPH_ROTL32(V16, 1); \
                V17 = SPH_ROTL32(V17, 1); \
                V24 = SPH_ROTL32(V24, 2); \
                V25 = SPH_ROTL32(V25, 2); \
                V26 = SPH_ROTL32(V26, 2); \
                V27 = SPH_ROTL32(V27, 2); \
                V34 = SPH_ROTL32(V34, 3); \
                V35 = SPH_ROTL32(V35, 3); \
                V36 = SPH_ROTL32(V36, 3); \
                V37 = SPH_ROTL32(V37, 3); \
        } while (0)


#define luffa_P4   do { \
                int r; \
                sph_u64 W0, W1, W2, W3, W4, W5, W6, W7; \
                TWEAK4; \
                W0 = (sph_u64)V00 | ((sph_u64)V10 << 32); \
                W1 = (sph_u64)V01 | ((sph_u64)V11 << 32); \
                W2 = (sph_u64)V02 | ((sph_u64)V12 << 32); \
                W3 = (sph_u64)V03 | ((sph_u64)V13 << 32); \
                W4 = (sph_u64)V04 | ((sph_u64)V14 << 32); \
                W5 = (sph_u64)V05 | ((sph_u64)V15 << 32); \
                W6 = (sph_u64)V06 | ((sph_u64)V16 << 32); \
                W7 = (sph_u64)V07 | ((sph_u64)V17 << 32); \
                for (r = 0; r < 8; r ++) { \
                        SUB_CRUMBW(W0, W1, W2, W3); \
                        SUB_CRUMBW(W5, W6, W7, W4); \
                        MIX_WORDW(W0, W4); \
                        MIX_WORDW(W1, W5); \
                        MIX_WORDW(W2, W6); \
                        MIX_WORDW(W3, W7); \
                        W0 ^= RCW010[r]; \
                        W4 ^= RCW014[r]; \
                } \
                V00 = SPH_T32((sph_u32)W0); \
                V10 = SPH_T32((sph_u32)(W0 >> 32)); \
                V01 = SPH_T32((sph_u32)W1); \
                V11 = SPH_T32((sph_u32)(W1 >> 32)); \
                V02 = SPH_T32((sph_u32)W2); \
                V12 = SPH_T32((sph_u32)(W2 >> 32)); \
                V03 = SPH_T32((sph_u32)W3); \
                V13 = SPH_T32((sph_u32)(W3 >> 32)); \
                V04 = SPH_T32((sph_u32)W4); \
                V14 = SPH_T32((sph_u32)(W4 >> 32)); \
                V05 = SPH_T32((sph_u32)W5); \
                V15 = SPH_T32((sph_u32)(W5 >> 32)); \
                V06 = SPH_T32((sph_u32)W6); \
                V16 = SPH_T32((sph_u32)(W6 >> 32)); \
                V07 = SPH_T32((sph_u32)W7); \
                V17 = SPH_T32((sph_u32)(W7 >> 32)); \
                W0 = (sph_u64)V20 | ((sph_u64)V30 << 32); \
                W1 = (sph_u64)V21 | ((sph_u64)V31 << 32); \
                W2 = (sph_u64)V22 | ((sph_u64)V32 << 32); \
                W3 = (sph_u64)V23 | ((sph_u64)V33 << 32); \
                W4 = (sph_u64)V24 | ((sph_u64)V34 << 32); \
                W5 = (sph_u64)V25 | ((sph_u64)V35 << 32); \
                W6 = (sph_u64)V26 | ((sph_u64)V36 << 32); \
                W7 = (sph_u64)V27 | ((sph_u64)V37 << 32); \
                for (r = 0; r < 8; r ++) { \
                        SUB_CRUMBW(W0, W1, W2, W3); \
                        SUB_CRUMBW(W5, W6, W7, W4); \
                        MIX_WORDW(W0, W4); \
                        MIX_WORDW(W1, W5); \
                        MIX_WORDW(W2, W6); \
                        MIX_WORDW(W3, W7); \
                        W0 ^= RCW230[r]; \
                        W4 ^= RCW234[r]; \
                } \
                V20 = SPH_T32((sph_u32)W0); \
                V30 = SPH_T32((sph_u32)(W0 >> 32)); \
                V21 = SPH_T32((sph_u32)W1); \
                V31 = SPH_T32((sph_u32)(W1 >> 32)); \
                V22 = SPH_T32((sph_u32)W2); \
                V32 = SPH_T32((sph_u32)(W2 >> 32)); \
                V23 = SPH_T32((sph_u32)W3); \
                V33 = SPH_T32((sph_u32)(W3 >> 32)); \
                V24 = SPH_T32((sph_u32)W4); \
                V34 = SPH_T32((sph_u32)(W4 >> 32)); \
                V25 = SPH_T32((sph_u32)W5); \
                V35 = SPH_T32((sph_u32)(W5 >> 32)); \
                V26 = SPH_T32((sph_u32)W6); \
                V36 = SPH_T32((sph_u32)(W6 >> 32)); \
                V27 = SPH_T32((sph_u32)W7); \
                V37 = SPH_T32((sph_u32)(W7 >> 32)); \
        } while (0)


#define luffa_DECL_STATE5 \
        sph_u32 V00, V01, V02, V03, V04, V05, V06, V07; \
        sph_u32 V10, V11, V12, V13, V14, V15, V16, V17; \
        sph_u32 V20, V21, V22, V23, V24, V25, V26, V27; \
        sph_u32 V30, V31, V32, V33, V34, V35, V36, V37; \
        sph_u32 V40, V41, V42, V43, V44, V45, V46, V47;

#define luffa_READ_STATE5(state)   do { \
                V00 = (state)->V[0][0]; \
                V01 = (state)->V[0][1]; \
                V02 = (state)->V[0][2]; \
                V03 = (state)->V[0][3]; \
                V04 = (state)->V[0][4]; \
                V05 = (state)->V[0][5]; \
                V06 = (state)->V[0][6]; \
                V07 = (state)->V[0][7]; \
                V10 = (state)->V[1][0]; \
                V11 = (state)->V[1][1]; \
                V12 = (state)->V[1][2]; \
                V13 = (state)->V[1][3]; \
                V14 = (state)->V[1][4]; \
                V15 = (state)->V[1][5]; \
                V16 = (state)->V[1][6]; \
                V17 = (state)->V[1][7]; \
                V20 = (state)->V[2][0]; \
                V21 = (state)->V[2][1]; \
                V22 = (state)->V[2][2]; \
                V23 = (state)->V[2][3]; \
                V24 = (state)->V[2][4]; \
                V25 = (state)->V[2][5]; \
                V26 = (state)->V[2][6]; \
                V27 = (state)->V[2][7]; \
                V30 = (state)->V[3][0]; \
                V31 = (state)->V[3][1]; \
                V32 = (state)->V[3][2]; \
                V33 = (state)->V[3][3]; \
                V34 = (state)->V[3][4]; \
                V35 = (state)->V[3][5]; \
                V36 = (state)->V[3][6]; \
                V37 = (state)->V[3][7]; \
                V40 = (state)->V[4][0]; \
                V41 = (state)->V[4][1]; \
                V42 = (state)->V[4][2]; \
                V43 = (state)->V[4][3]; \
                V44 = (state)->V[4][4]; \
                V45 = (state)->V[4][5]; \
                V46 = (state)->V[4][6]; \
                V47 = (state)->V[4][7]; \
        } while (0)

#define luffa_WRITE_STATE5(state)   do { \
                (state)->V[0][0] = V00; \
                (state)->V[0][1] = V01; \
                (state)->V[0][2] = V02; \
                (state)->V[0][3] = V03; \
                (state)->V[0][4] = V04; \
                (state)->V[0][5] = V05; \
                (state)->V[0][6] = V06; \
                (state)->V[0][7] = V07; \
                (state)->V[1][0] = V10; \
                (state)->V[1][1] = V11; \
                (state)->V[1][2] = V12; \
                (state)->V[1][3] = V13; \
                (state)->V[1][4] = V14; \
                (state)->V[1][5] = V15; \
                (state)->V[1][6] = V16; \
                (state)->V[1][7] = V17; \
                (state)->V[2][0] = V20; \
                (state)->V[2][1] = V21; \
                (state)->V[2][2] = V22; \
                (state)->V[2][3] = V23; \
                (state)->V[2][4] = V24; \
                (state)->V[2][5] = V25; \
                (state)->V[2][6] = V26; \
                (state)->V[2][7] = V27; \
                (state)->V[3][0] = V30; \
                (state)->V[3][1] = V31; \
                (state)->V[3][2] = V32; \
                (state)->V[3][3] = V33; \
                (state)->V[3][4] = V34; \
                (state)->V[3][5] = V35; \
                (state)->V[3][6] = V36; \
                (state)->V[3][7] = V37; \
                (state)->V[4][0] = V40; \
                (state)->V[4][1] = V41; \
                (state)->V[4][2] = V42; \
                (state)->V[4][3] = V43; \
                (state)->V[4][4] = V44; \
                (state)->V[4][5] = V45; \
                (state)->V[4][6] = V46; \
                (state)->V[4][7] = V47; \
        } while (0)

#define MI5   do { \
                DECL_TMP8(M) \
                DECL_TMP8(a) \
                DECL_TMP8(b) \
                M0 = sph_dec32be_aligned(buf +  0); \
                M1 = sph_dec32be_aligned(buf +  4); \
                M2 = sph_dec32be_aligned(buf +  8); \
                M3 = sph_dec32be_aligned(buf + 12); \
                M4 = sph_dec32be_aligned(buf + 16); \
                M5 = sph_dec32be_aligned(buf + 20); \
                M6 = sph_dec32be_aligned(buf + 24); \
                M7 = sph_dec32be_aligned(buf + 28); \
                XOR(a, V0, V1); \
                XOR(b, V2, V3); \
                XOR(a, a, b); \
                XOR(a, a, V4); \
                M2(a, a); \
                XOR(V0, a, V0); \
                XOR(V1, a, V1); \
                XOR(V2, a, V2); \
                XOR(V3, a, V3); \
                XOR(V4, a, V4); \
                M2(b, V0); \
                XOR(b, b, V1); \
                M2(V1, V1); \
                XOR(V1, V1, V2); \
                M2(V2, V2); \
                XOR(V2, V2, V3); \
                M2(V3, V3); \
                XOR(V3, V3, V4); \
                M2(V4, V4); \
                XOR(V4, V4, V0); \
                M2(V0, b); \
                XOR(V0, V0, V4); \
                M2(V4, V4); \
                XOR(V4, V4, V3); \
                M2(V3, V3); \
                XOR(V3, V3, V2); \
                M2(V2, V2); \
                XOR(V2, V2, V1); \
                M2(V1, V1); \
                XOR(V1, V1, b); \
                XOR(V0, V0, M); \
                M2(M, M); \
                XOR(V1, V1, M); \
                M2(M, M); \
                XOR(V2, V2, M); \
                M2(M, M); \
                XOR(V3, V3, M); \
                M2(M, M); \
                XOR(V4, V4, M); \
        } while (0)

#define TWEAK5   do { \
                V14 = SPH_ROTL32(V14, 1); \
                V15 = SPH_ROTL32(V15, 1); \
                V16 = SPH_ROTL32(V16, 1); \
                V17 = SPH_ROTL32(V17, 1); \
                V24 = SPH_ROTL32(V24, 2); \
                V25 = SPH_ROTL32(V25, 2); \
                V26 = SPH_ROTL32(V26, 2); \
                V27 = SPH_ROTL32(V27, 2); \
                V34 = SPH_ROTL32(V34, 3); \
                V35 = SPH_ROTL32(V35, 3); \
                V36 = SPH_ROTL32(V36, 3); \
                V37 = SPH_ROTL32(V37, 3); \
                V44 = SPH_ROTL32(V44, 4); \
                V45 = SPH_ROTL32(V45, 4); \
                V46 = SPH_ROTL32(V46, 4); \
                V47 = SPH_ROTL32(V47, 4); \
        } while (0)


#define luffa_P5   do { \
                int r; \
                sph_u64 W0, W1, W2, W3, W4, W5, W6, W7; \
                TWEAK5; \
                W0 = (sph_u64)V00 | ((sph_u64)V10 << 32); \
                W1 = (sph_u64)V01 | ((sph_u64)V11 << 32); \
                W2 = (sph_u64)V02 | ((sph_u64)V12 << 32); \
                W3 = (sph_u64)V03 | ((sph_u64)V13 << 32); \
                W4 = (sph_u64)V04 | ((sph_u64)V14 << 32); \
                W5 = (sph_u64)V05 | ((sph_u64)V15 << 32); \
                W6 = (sph_u64)V06 | ((sph_u64)V16 << 32); \
                W7 = (sph_u64)V07 | ((sph_u64)V17 << 32); \
                for (r = 0; r < 8; r ++) { \
                        SUB_CRUMBW(W0, W1, W2, W3); \
                        SUB_CRUMBW(W5, W6, W7, W4); \
                        MIX_WORDW(W0, W4); \
                        MIX_WORDW(W1, W5); \
                        MIX_WORDW(W2, W6); \
                        MIX_WORDW(W3, W7); \
                        W0 ^= RCW010[r]; \
                        W4 ^= RCW014[r]; \
                } \
                V00 = SPH_T32((sph_u32)W0); \
                V10 = SPH_T32((sph_u32)(W0 >> 32)); \
                V01 = SPH_T32((sph_u32)W1); \
                V11 = SPH_T32((sph_u32)(W1 >> 32)); \
                V02 = SPH_T32((sph_u32)W2); \
                V12 = SPH_T32((sph_u32)(W2 >> 32)); \
                V03 = SPH_T32((sph_u32)W3); \
                V13 = SPH_T32((sph_u32)(W3 >> 32)); \
                V04 = SPH_T32((sph_u32)W4); \
                V14 = SPH_T32((sph_u32)(W4 >> 32)); \
                V05 = SPH_T32((sph_u32)W5); \
                V15 = SPH_T32((sph_u32)(W5 >> 32)); \
                V06 = SPH_T32((sph_u32)W6); \
                V16 = SPH_T32((sph_u32)(W6 >> 32)); \
                V07 = SPH_T32((sph_u32)W7); \
                V17 = SPH_T32((sph_u32)(W7 >> 32)); \
                W0 = (sph_u64)V20 | ((sph_u64)V30 << 32); \
                W1 = (sph_u64)V21 | ((sph_u64)V31 << 32); \
                W2 = (sph_u64)V22 | ((sph_u64)V32 << 32); \
                W3 = (sph_u64)V23 | ((sph_u64)V33 << 32); \
                W4 = (sph_u64)V24 | ((sph_u64)V34 << 32); \
                W5 = (sph_u64)V25 | ((sph_u64)V35 << 32); \
                W6 = (sph_u64)V26 | ((sph_u64)V36 << 32); \
                W7 = (sph_u64)V27 | ((sph_u64)V37 << 32); \
                for (r = 0; r < 8; r ++) { \
                        SUB_CRUMBW(W0, W1, W2, W3); \
                        SUB_CRUMBW(W5, W6, W7, W4); \
                        MIX_WORDW(W0, W4); \
                        MIX_WORDW(W1, W5); \
                        MIX_WORDW(W2, W6); \
                        MIX_WORDW(W3, W7); \
                        W0 ^= RCW230[r]; \
                        W4 ^= RCW234[r]; \
                } \
                V20 = SPH_T32((sph_u32)W0); \
                V30 = SPH_T32((sph_u32)(W0 >> 32)); \
                V21 = SPH_T32((sph_u32)W1); \
                V31 = SPH_T32((sph_u32)(W1 >> 32)); \
                V22 = SPH_T32((sph_u32)W2); \
                V32 = SPH_T32((sph_u32)(W2 >> 32)); \
                V23 = SPH_T32((sph_u32)W3); \
                V33 = SPH_T32((sph_u32)(W3 >> 32)); \
                V24 = SPH_T32((sph_u32)W4); \
                V34 = SPH_T32((sph_u32)(W4 >> 32)); \
                V25 = SPH_T32((sph_u32)W5); \
                V35 = SPH_T32((sph_u32)(W5 >> 32)); \
                V26 = SPH_T32((sph_u32)W6); \
                V36 = SPH_T32((sph_u32)(W6 >> 32)); \
                V27 = SPH_T32((sph_u32)W7); \
                V37 = SPH_T32((sph_u32)(W7 >> 32)); \
                for (r = 0; r < 8; r ++) { \
                        SUB_CRUMB(V40, V41, V42, V43); \
                        SUB_CRUMB(V45, V46, V47, V44); \
                        MIX_WORD(V40, V44); \
                        MIX_WORD(V41, V45); \
                        MIX_WORD(V42, V46); \
                        MIX_WORD(V43, V47); \
                        V40 ^= RC40[r]; \
                        V44 ^= RC44[r]; \
                } \
        } while (0)


static void
luffa3(sph_luffa224_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;
        luffa_DECL_STATE3

        buf = sc->buf;
        ptr = sc->ptr;
        if (len < (sizeof sc->buf) - ptr) {
                memcpy(buf + ptr, data, len);
                ptr += len;
                sc->ptr = ptr;
                return;
        }

        luffa_READ_STATE3(sc);
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                ptr += clen;
                data = (const unsigned char *)data + clen;
                len -= clen;
                if (ptr == sizeof sc->buf) {
                        MI3;
                        luffa_P3;
                        ptr = 0;
                }
        }
        luffa_WRITE_STATE3(sc);
        sc->ptr = ptr;
}

static void
luffa3_close(sph_luffa224_context *sc, unsigned ub, unsigned n,
    unsigned char *dst, unsigned out_size_w32)
{
        unsigned char *buf, *out;
        size_t ptr;
        unsigned z;
        int i;
        luffa_DECL_STATE3

        buf = sc->buf;
        ptr = sc->ptr;
        z = 0x80 >> n;
        buf[ptr ++] = ((ub & -z) | z) & 0xFF;
        memset(buf + ptr, 0, (sizeof sc->buf) - ptr);
        luffa_READ_STATE3(sc);
        for (i = 0; i < 2; i ++) {
                MI3;
                luffa_P3;
                memset(buf, 0, sizeof sc->buf);
        }
        out = dst;
        sph_enc32be(out +  0, V00 ^ V10 ^ V20);
        sph_enc32be(out +  4, V01 ^ V11 ^ V21);
        sph_enc32be(out +  8, V02 ^ V12 ^ V22);
        sph_enc32be(out + 12, V03 ^ V13 ^ V23);
        sph_enc32be(out + 16, V04 ^ V14 ^ V24);
        sph_enc32be(out + 20, V05 ^ V15 ^ V25);
        sph_enc32be(out + 24, V06 ^ V16 ^ V26);
        if (out_size_w32 > 7)
                sph_enc32be(out + 28, V07 ^ V17 ^ V27);
}

static void
luffa4(sph_luffa384_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;
        luffa_DECL_STATE4

        buf = sc->buf;
        ptr = sc->ptr;
        if (len < (sizeof sc->buf) - ptr) {
                memcpy(buf + ptr, data, len);
                ptr += len;
                sc->ptr = ptr;
                return;
        }

        luffa_READ_STATE4(sc);
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                ptr += clen;
                data = (const unsigned char *)data + clen;
                len -= clen;
                if (ptr == sizeof sc->buf) {
                        MI4;
                        luffa_P4;
                        ptr = 0;
                }
        }
        luffa_WRITE_STATE4(sc);
        sc->ptr = ptr;
}

static void
luffa4_close(sph_luffa384_context *sc, unsigned ub, unsigned n, unsigned char *dst)
{
        unsigned char *buf, *out;
        size_t ptr;
        unsigned z;
        int i;
        luffa_DECL_STATE4

        buf = sc->buf;
        ptr = sc->ptr;
        out = dst;
        z = 0x80 >> n;
        buf[ptr ++] = ((ub & -z) | z) & 0xFF;
        memset(buf + ptr, 0, (sizeof sc->buf) - ptr);
        luffa_READ_STATE4(sc);
        for (i = 0; i < 3; i ++) {
                MI4;
                luffa_P4;
                switch (i) {
                case 0:
                        memset(buf, 0, sizeof sc->buf);
                        break;
                case 1:
                        sph_enc32be(out +  0, V00 ^ V10 ^ V20 ^ V30);
                        sph_enc32be(out +  4, V01 ^ V11 ^ V21 ^ V31);
                        sph_enc32be(out +  8, V02 ^ V12 ^ V22 ^ V32);
                        sph_enc32be(out + 12, V03 ^ V13 ^ V23 ^ V33);
                        sph_enc32be(out + 16, V04 ^ V14 ^ V24 ^ V34);
                        sph_enc32be(out + 20, V05 ^ V15 ^ V25 ^ V35);
                        sph_enc32be(out + 24, V06 ^ V16 ^ V26 ^ V36);
                        sph_enc32be(out + 28, V07 ^ V17 ^ V27 ^ V37);
                        break;
                case 2:
                        sph_enc32be(out + 32, V00 ^ V10 ^ V20 ^ V30);
                        sph_enc32be(out + 36, V01 ^ V11 ^ V21 ^ V31);
                        sph_enc32be(out + 40, V02 ^ V12 ^ V22 ^ V32);
                        sph_enc32be(out + 44, V03 ^ V13 ^ V23 ^ V33);
                        break;
                }
        }
}

static void
luffa5(sph_luffa512_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;
        luffa_DECL_STATE5

        buf = sc->buf;
        ptr = sc->ptr;
        if (len < (sizeof sc->buf) - ptr) {
                memcpy(buf + ptr, data, len);
                ptr += len;
                sc->ptr = ptr;
                return;
        }

        luffa_READ_STATE5(sc);
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                ptr += clen;
                data = (const unsigned char *)data + clen;
                len -= clen;
                if (ptr == sizeof sc->buf) {
                        MI5;
                        luffa_P5;
                        ptr = 0;
                }
        }
        luffa_WRITE_STATE5(sc);
        sc->ptr = ptr;
}

static void
luffa5_close(sph_luffa512_context *sc, unsigned ub, unsigned n, unsigned char *dst)
{
        unsigned char *buf, *out;
        size_t ptr;
        unsigned z;
        int i;
        luffa_DECL_STATE5

        buf = sc->buf;
        ptr = sc->ptr;
        out = dst;
        z = 0x80 >> n;
        buf[ptr ++] = ((ub & -z) | z) & 0xFF;
        memset(buf + ptr, 0, (sizeof sc->buf) - ptr);
        luffa_READ_STATE5(sc);
        for (i = 0; i < 3; i ++) {
                MI5;
                luffa_P5;
                switch (i) {
                case 0:
                        memset(buf, 0, sizeof sc->buf);
                        break;
                case 1:
                        sph_enc32be(out +  0, V00 ^ V10 ^ V20 ^ V30 ^ V40);
                        sph_enc32be(out +  4, V01 ^ V11 ^ V21 ^ V31 ^ V41);
                        sph_enc32be(out +  8, V02 ^ V12 ^ V22 ^ V32 ^ V42);
                        sph_enc32be(out + 12, V03 ^ V13 ^ V23 ^ V33 ^ V43);
                        sph_enc32be(out + 16, V04 ^ V14 ^ V24 ^ V34 ^ V44);
                        sph_enc32be(out + 20, V05 ^ V15 ^ V25 ^ V35 ^ V45);
                        sph_enc32be(out + 24, V06 ^ V16 ^ V26 ^ V36 ^ V46);
                        sph_enc32be(out + 28, V07 ^ V17 ^ V27 ^ V37 ^ V47);
                        break;
                case 2:
                        sph_enc32be(out + 32, V00 ^ V10 ^ V20 ^ V30 ^ V40);
                        sph_enc32be(out + 36, V01 ^ V11 ^ V21 ^ V31 ^ V41);
                        sph_enc32be(out + 40, V02 ^ V12 ^ V22 ^ V32 ^ V42);
                        sph_enc32be(out + 44, V03 ^ V13 ^ V23 ^ V33 ^ V43);
                        sph_enc32be(out + 48, V04 ^ V14 ^ V24 ^ V34 ^ V44);
                        sph_enc32be(out + 52, V05 ^ V15 ^ V25 ^ V35 ^ V45);
                        sph_enc32be(out + 56, V06 ^ V16 ^ V26 ^ V36 ^ V46);
                        sph_enc32be(out + 60, V07 ^ V17 ^ V27 ^ V37 ^ V47);
                        break;
                }
        }
}

void
sph_luffa224_init(sph_luffa224_context *cc)
{
        sph_luffa224_context *sc;

        sc = cc;
        memcpy(sc->V, V_INIT, sizeof(sc->V));
        sc->ptr = 0;
}

void
sph_luffa224(sph_luffa224_context *cc, const unsigned char *data, size_t len)
{
        luffa3(cc, data, len);
}

void
sph_luffa224_close(sph_luffa224_context *cc, unsigned char *dst)
{
        sph_luffa224_addbits_and_close(cc, 0, 0, dst);
}

void
sph_luffa224_addbits_and_close(sph_luffa224_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        luffa3_close(cc, ub, n, dst, 7);
        sph_luffa224_init(cc);
}

void
sph_luffa256_init(sph_luffa256_context *cc)
{
        sph_luffa256_context *sc;

        sc = cc;
        memcpy(sc->V, V_INIT, sizeof(sc->V));
        sc->ptr = 0;
}

void
sph_luffa256(sph_luffa256_context *cc, const unsigned char *data, size_t len)
{
        luffa3(cc, data, len);
}

void
sph_luffa256_close(sph_luffa256_context *cc, unsigned char *dst)
{
        sph_luffa256_addbits_and_close(cc, 0, 0, dst);
}

void
sph_luffa256_addbits_and_close(sph_luffa256_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        luffa3_close(cc, ub, n, dst, 8);
        sph_luffa256_init(cc);
}

void
sph_luffa384_init(sph_luffa384_context *cc)
{
        sph_luffa384_context *sc;

        sc = cc;
        memcpy(sc->V, V_INIT, sizeof(sc->V));
        sc->ptr = 0;
}

void
sph_luffa384(sph_luffa384_context *cc, const unsigned char *data, size_t len)
{
        luffa4(cc, data, len);
}

void
sph_luffa384_close(sph_luffa384_context *cc, unsigned char *dst)
{
        sph_luffa384_addbits_and_close(cc, 0, 0, dst);
}

void
sph_luffa384_addbits_and_close(sph_luffa384_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        luffa4_close(cc, ub, n, dst);
        sph_luffa384_init(cc);
}

void
sph_luffa512_init(sph_luffa512_context *cc)
{
        sph_luffa512_context *sc;

        sc = cc;
        memcpy(sc->V, V_INIT, sizeof(sc->V));
        sc->ptr = 0;
}

void
sph_luffa512(sph_luffa512_context *cc, const unsigned char *data, size_t len)
{
        luffa5(cc, data, len);
}

void
sph_luffa512_close(sph_luffa512_context *cc, unsigned char *dst)
{
        sph_luffa512_addbits_and_close(cc, 0, 0, dst);
}

void
sph_luffa512_addbits_and_close(sph_luffa512_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        luffa5_close(cc, ub, n, dst);
        sph_luffa512_init(cc);
}













#define SPH_SIZE_shabal192   192
#define SPH_SIZE_shabal224   224
#define SPH_SIZE_shabal256   256
#define SPH_SIZE_shabal384   384
#define SPH_SIZE_shabal512   512

typedef struct {
        unsigned char buf[64];    /* first field, for alignment */
        size_t ptr;
        sph_u32 A[12], B[16], C[16];
        sph_u32 Whigh, Wlow;
} sph_shabal_context;

void
sph_shabal192_init(sph_shabal_context *cc);
void
sph_shabal192(sph_shabal_context *cc, const unsigned char *data, size_t len);
void
sph_shabal192_close(sph_shabal_context *cc, unsigned char *dst);
void
sph_shabal192_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_shabal224_init(sph_shabal_context *cc);
void
sph_shabal224(sph_shabal_context *cc, const unsigned char *data, size_t len);
void
sph_shabal224_close(sph_shabal_context *cc, unsigned char *dst);
void
sph_shabal224_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_shabal256_init(sph_shabal_context *cc);
void
sph_shabal256(sph_shabal_context *cc, const unsigned char *data, size_t len);
void
sph_shabal256_close(sph_shabal_context *cc, unsigned char *dst);
void
sph_shabal256_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_shabal384_init(sph_shabal_context *cc);
void
sph_shabal384(sph_shabal_context *cc, const unsigned char *data, size_t len);
void
sph_shabal384_close(sph_shabal_context *cc, unsigned char *dst);
void
sph_shabal384_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_shabal512_init(sph_shabal_context *cc);
void
sph_shabal512(sph_shabal_context *cc, const unsigned char *data, size_t len);
void
sph_shabal512_close(sph_shabal_context *cc, unsigned char *dst);
void
sph_shabal512_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst);











#define sM    16
#define C32   SPH_C32
#define T32   SPH_T32
#define O1   13
#define O2    9
#define O3    6


/* BEGIN -- automatically generated code. */

#define shabal_DECL_STATE   \
        sph_u32 A00, A01, A02, A03, A04, A05, A06, A07, \
                A08, A09, A0A, A0B; \
        sph_u32 B0, B1, B2, B3, B4, B5, B6, B7, \
                B8, B9, BA, BB, BC, BD, BE, BF; \
        sph_u32 C0, C1, C2, C3, C4, C5, C6, C7, \
                C8, C9, CA, CB, CC, CD, CE, CF; \
        sph_u32 M0, M1, M2, M3, M4, M5, M6, M7, \
                M8, M9, MA, MB, MC, MD, ME, MF; \
        sph_u32 Wlow, Whigh;

#define shabal_READ_STATE(state)   do { \
                A00 = (state)->A[0]; \
                A01 = (state)->A[1]; \
                A02 = (state)->A[2]; \
                A03 = (state)->A[3]; \
                A04 = (state)->A[4]; \
                A05 = (state)->A[5]; \
                A06 = (state)->A[6]; \
                A07 = (state)->A[7]; \
                A08 = (state)->A[8]; \
                A09 = (state)->A[9]; \
                A0A = (state)->A[10]; \
                A0B = (state)->A[11]; \
                B0 = (state)->B[0]; \
                B1 = (state)->B[1]; \
                B2 = (state)->B[2]; \
                B3 = (state)->B[3]; \
                B4 = (state)->B[4]; \
                B5 = (state)->B[5]; \
                B6 = (state)->B[6]; \
                B7 = (state)->B[7]; \
                B8 = (state)->B[8]; \
                B9 = (state)->B[9]; \
                BA = (state)->B[10]; \
                BB = (state)->B[11]; \
                BC = (state)->B[12]; \
                BD = (state)->B[13]; \
                BE = (state)->B[14]; \
                BF = (state)->B[15]; \
                C0 = (state)->C[0]; \
                C1 = (state)->C[1]; \
                C2 = (state)->C[2]; \
                C3 = (state)->C[3]; \
                C4 = (state)->C[4]; \
                C5 = (state)->C[5]; \
                C6 = (state)->C[6]; \
                C7 = (state)->C[7]; \
                C8 = (state)->C[8]; \
                C9 = (state)->C[9]; \
                CA = (state)->C[10]; \
                CB = (state)->C[11]; \
                CC = (state)->C[12]; \
                CD = (state)->C[13]; \
                CE = (state)->C[14]; \
                CF = (state)->C[15]; \
                Wlow = (state)->Wlow; \
                Whigh = (state)->Whigh; \
        } while (0)

#define shabal_WRITE_STATE(state)   do { \
                (state)->A[0] = A00; \
                (state)->A[1] = A01; \
                (state)->A[2] = A02; \
                (state)->A[3] = A03; \
                (state)->A[4] = A04; \
                (state)->A[5] = A05; \
                (state)->A[6] = A06; \
                (state)->A[7] = A07; \
                (state)->A[8] = A08; \
                (state)->A[9] = A09; \
                (state)->A[10] = A0A; \
                (state)->A[11] = A0B; \
                (state)->B[0] = B0; \
                (state)->B[1] = B1; \
                (state)->B[2] = B2; \
                (state)->B[3] = B3; \
                (state)->B[4] = B4; \
                (state)->B[5] = B5; \
                (state)->B[6] = B6; \
                (state)->B[7] = B7; \
                (state)->B[8] = B8; \
                (state)->B[9] = B9; \
                (state)->B[10] = BA; \
                (state)->B[11] = BB; \
                (state)->B[12] = BC; \
                (state)->B[13] = BD; \
                (state)->B[14] = BE; \
                (state)->B[15] = BF; \
                (state)->C[0] = C0; \
                (state)->C[1] = C1; \
                (state)->C[2] = C2; \
                (state)->C[3] = C3; \
                (state)->C[4] = C4; \
                (state)->C[5] = C5; \
                (state)->C[6] = C6; \
                (state)->C[7] = C7; \
                (state)->C[8] = C8; \
                (state)->C[9] = C9; \
                (state)->C[10] = CA; \
                (state)->C[11] = CB; \
                (state)->C[12] = CC; \
                (state)->C[13] = CD; \
                (state)->C[14] = CE; \
                (state)->C[15] = CF; \
                (state)->Wlow = Wlow; \
                (state)->Whigh = Whigh; \
        } while (0)

#define DECODE_BLOCK   do { \
                M0 = sph_dec32le_aligned(buf + 0); \
                M1 = sph_dec32le_aligned(buf + 4); \
                M2 = sph_dec32le_aligned(buf + 8); \
                M3 = sph_dec32le_aligned(buf + 12); \
                M4 = sph_dec32le_aligned(buf + 16); \
                M5 = sph_dec32le_aligned(buf + 20); \
                M6 = sph_dec32le_aligned(buf + 24); \
                M7 = sph_dec32le_aligned(buf + 28); \
                M8 = sph_dec32le_aligned(buf + 32); \
                M9 = sph_dec32le_aligned(buf + 36); \
                MA = sph_dec32le_aligned(buf + 40); \
                MB = sph_dec32le_aligned(buf + 44); \
                MC = sph_dec32le_aligned(buf + 48); \
                MD = sph_dec32le_aligned(buf + 52); \
                ME = sph_dec32le_aligned(buf + 56); \
                MF = sph_dec32le_aligned(buf + 60); \
        } while (0)

#define INPUT_BLOCK_ADD   do { \
                B0 = T32(B0 + M0); \
                B1 = T32(B1 + M1); \
                B2 = T32(B2 + M2); \
                B3 = T32(B3 + M3); \
                B4 = T32(B4 + M4); \
                B5 = T32(B5 + M5); \
                B6 = T32(B6 + M6); \
                B7 = T32(B7 + M7); \
                B8 = T32(B8 + M8); \
                B9 = T32(B9 + M9); \
                BA = T32(BA + MA); \
                BB = T32(BB + MB); \
                BC = T32(BC + MC); \
                BD = T32(BD + MD); \
                BE = T32(BE + ME); \
                BF = T32(BF + MF); \
        } while (0)

#define INPUT_BLOCK_SUB   do { \
                C0 = T32(C0 - M0); \
                C1 = T32(C1 - M1); \
                C2 = T32(C2 - M2); \
                C3 = T32(C3 - M3); \
                C4 = T32(C4 - M4); \
                C5 = T32(C5 - M5); \
                C6 = T32(C6 - M6); \
                C7 = T32(C7 - M7); \
                C8 = T32(C8 - M8); \
                C9 = T32(C9 - M9); \
                CA = T32(CA - MA); \
                CB = T32(CB - MB); \
                CC = T32(CC - MC); \
                CD = T32(CD - MD); \
                CE = T32(CE - ME); \
                CF = T32(CF - MF); \
        } while (0)

#define XOR_W   do { \
                A00 ^= Wlow; \
                A01 ^= Whigh; \
        } while (0)

#define SWAP(v1, v2)   do { \
                sph_u32 tmp = (v1); \
                (v1) = (v2); \
                (v2) = tmp; \
        } while (0)

#define SWAP_BC   do { \
                SWAP(B0, C0); \
                SWAP(B1, C1); \
                SWAP(B2, C2); \
                SWAP(B3, C3); \
                SWAP(B4, C4); \
                SWAP(B5, C5); \
                SWAP(B6, C6); \
                SWAP(B7, C7); \
                SWAP(B8, C8); \
                SWAP(B9, C9); \
                SWAP(BA, CA); \
                SWAP(BB, CB); \
                SWAP(BC, CC); \
                SWAP(BD, CD); \
                SWAP(BE, CE); \
                SWAP(BF, CF); \
        } while (0)

#define PERM_ELT(xa0, xa1, xb0, xb1, xb2, xb3, xc, xm)   do { \
                xa0 = T32((xa0 \
                        ^ (((xa1 << 15) | (xa1 >> 17)) * 5U) \
                        ^ xc) * 3U) \
                        ^ xb1 ^ (xb2 & ~xb3) ^ xm; \
                xb0 = T32(~(((xb0 << 1) | (xb0 >> 31)) ^ xa0)); \
        } while (0)

#define PERM_STEP_0   do { \
                PERM_ELT(A00, A0B, B0, BD, B9, B6, C8, M0); \
                PERM_ELT(A01, A00, B1, BE, BA, B7, C7, M1); \
                PERM_ELT(A02, A01, B2, BF, BB, B8, C6, M2); \
                PERM_ELT(A03, A02, B3, B0, BC, B9, C5, M3); \
                PERM_ELT(A04, A03, B4, B1, BD, BA, C4, M4); \
                PERM_ELT(A05, A04, B5, B2, BE, BB, C3, M5); \
                PERM_ELT(A06, A05, B6, B3, BF, BC, C2, M6); \
                PERM_ELT(A07, A06, B7, B4, B0, BD, C1, M7); \
                PERM_ELT(A08, A07, B8, B5, B1, BE, C0, M8); \
                PERM_ELT(A09, A08, B9, B6, B2, BF, CF, M9); \
                PERM_ELT(A0A, A09, BA, B7, B3, B0, CE, MA); \
                PERM_ELT(A0B, A0A, BB, B8, B4, B1, CD, MB); \
                PERM_ELT(A00, A0B, BC, B9, B5, B2, CC, MC); \
                PERM_ELT(A01, A00, BD, BA, B6, B3, CB, MD); \
                PERM_ELT(A02, A01, BE, BB, B7, B4, CA, ME); \
                PERM_ELT(A03, A02, BF, BC, B8, B5, C9, MF); \
        } while (0)

#define PERM_STEP_1   do { \
                PERM_ELT(A04, A03, B0, BD, B9, B6, C8, M0); \
                PERM_ELT(A05, A04, B1, BE, BA, B7, C7, M1); \
                PERM_ELT(A06, A05, B2, BF, BB, B8, C6, M2); \
                PERM_ELT(A07, A06, B3, B0, BC, B9, C5, M3); \
                PERM_ELT(A08, A07, B4, B1, BD, BA, C4, M4); \
                PERM_ELT(A09, A08, B5, B2, BE, BB, C3, M5); \
                PERM_ELT(A0A, A09, B6, B3, BF, BC, C2, M6); \
                PERM_ELT(A0B, A0A, B7, B4, B0, BD, C1, M7); \
                PERM_ELT(A00, A0B, B8, B5, B1, BE, C0, M8); \
                PERM_ELT(A01, A00, B9, B6, B2, BF, CF, M9); \
                PERM_ELT(A02, A01, BA, B7, B3, B0, CE, MA); \
                PERM_ELT(A03, A02, BB, B8, B4, B1, CD, MB); \
                PERM_ELT(A04, A03, BC, B9, B5, B2, CC, MC); \
                PERM_ELT(A05, A04, BD, BA, B6, B3, CB, MD); \
                PERM_ELT(A06, A05, BE, BB, B7, B4, CA, ME); \
                PERM_ELT(A07, A06, BF, BC, B8, B5, C9, MF); \
        } while (0)

#define PERM_STEP_2   do { \
                PERM_ELT(A08, A07, B0, BD, B9, B6, C8, M0); \
                PERM_ELT(A09, A08, B1, BE, BA, B7, C7, M1); \
                PERM_ELT(A0A, A09, B2, BF, BB, B8, C6, M2); \
                PERM_ELT(A0B, A0A, B3, B0, BC, B9, C5, M3); \
                PERM_ELT(A00, A0B, B4, B1, BD, BA, C4, M4); \
                PERM_ELT(A01, A00, B5, B2, BE, BB, C3, M5); \
                PERM_ELT(A02, A01, B6, B3, BF, BC, C2, M6); \
                PERM_ELT(A03, A02, B7, B4, B0, BD, C1, M7); \
                PERM_ELT(A04, A03, B8, B5, B1, BE, C0, M8); \
                PERM_ELT(A05, A04, B9, B6, B2, BF, CF, M9); \
                PERM_ELT(A06, A05, BA, B7, B3, B0, CE, MA); \
                PERM_ELT(A07, A06, BB, B8, B4, B1, CD, MB); \
                PERM_ELT(A08, A07, BC, B9, B5, B2, CC, MC); \
                PERM_ELT(A09, A08, BD, BA, B6, B3, CB, MD); \
                PERM_ELT(A0A, A09, BE, BB, B7, B4, CA, ME); \
                PERM_ELT(A0B, A0A, BF, BC, B8, B5, C9, MF); \
        } while (0)

#define APPLY_P   do { \
                B0 = T32(B0 << 17) | (B0 >> 15); \
                B1 = T32(B1 << 17) | (B1 >> 15); \
                B2 = T32(B2 << 17) | (B2 >> 15); \
                B3 = T32(B3 << 17) | (B3 >> 15); \
                B4 = T32(B4 << 17) | (B4 >> 15); \
                B5 = T32(B5 << 17) | (B5 >> 15); \
                B6 = T32(B6 << 17) | (B6 >> 15); \
                B7 = T32(B7 << 17) | (B7 >> 15); \
                B8 = T32(B8 << 17) | (B8 >> 15); \
                B9 = T32(B9 << 17) | (B9 >> 15); \
                BA = T32(BA << 17) | (BA >> 15); \
                BB = T32(BB << 17) | (BB >> 15); \
                BC = T32(BC << 17) | (BC >> 15); \
                BD = T32(BD << 17) | (BD >> 15); \
                BE = T32(BE << 17) | (BE >> 15); \
                BF = T32(BF << 17) | (BF >> 15); \
                PERM_STEP_0; \
                PERM_STEP_1; \
                PERM_STEP_2; \
                A0B = T32(A0B + C6); \
                A0A = T32(A0A + C5); \
                A09 = T32(A09 + C4); \
                A08 = T32(A08 + C3); \
                A07 = T32(A07 + C2); \
                A06 = T32(A06 + C1); \
                A05 = T32(A05 + C0); \
                A04 = T32(A04 + CF); \
                A03 = T32(A03 + CE); \
                A02 = T32(A02 + CD); \
                A01 = T32(A01 + CC); \
                A00 = T32(A00 + CB); \
                A0B = T32(A0B + CA); \
                A0A = T32(A0A + C9); \
                A09 = T32(A09 + C8); \
                A08 = T32(A08 + C7); \
                A07 = T32(A07 + C6); \
                A06 = T32(A06 + C5); \
                A05 = T32(A05 + C4); \
                A04 = T32(A04 + C3); \
                A03 = T32(A03 + C2); \
                A02 = T32(A02 + C1); \
                A01 = T32(A01 + C0); \
                A00 = T32(A00 + CF); \
                A0B = T32(A0B + CE); \
                A0A = T32(A0A + CD); \
                A09 = T32(A09 + CC); \
                A08 = T32(A08 + CB); \
                A07 = T32(A07 + CA); \
                A06 = T32(A06 + C9); \
                A05 = T32(A05 + C8); \
                A04 = T32(A04 + C7); \
                A03 = T32(A03 + C6); \
                A02 = T32(A02 + C5); \
                A01 = T32(A01 + C4); \
                A00 = T32(A00 + C3); \
        } while (0)

#define INCR_W   do { \
                if ((Wlow = T32(Wlow + 1)) == 0) \
                        Whigh = T32(Whigh + 1); \
        } while (0)

static const sph_u32 A_init_192[] = {
        C32(0xFD749ED4), C32(0xB798E530), C32(0x33904B6F), C32(0x46BDA85E),
        C32(0x076934B4), C32(0x454B4058), C32(0x77F74527), C32(0xFB4CF465),
        C32(0x62931DA9), C32(0xE778C8DB), C32(0x22B3998E), C32(0xAC15CFB9)
};

static const sph_u32 B_init_192[] = {
        C32(0x58BCBAC4), C32(0xEC47A08E), C32(0xAEE933B2), C32(0xDFCBC824),
        C32(0xA7944804), C32(0xBF65BDB0), C32(0x5A9D4502), C32(0x59979AF7),
        C32(0xC5CEA54E), C32(0x4B6B8150), C32(0x16E71909), C32(0x7D632319),
        C32(0x930573A0), C32(0xF34C63D1), C32(0xCAF914B4), C32(0xFDD6612C)
};

static const sph_u32 C_init_192[] = {
        C32(0x61550878), C32(0x89EF2B75), C32(0xA1660C46), C32(0x7EF3855B),
        C32(0x7297B58C), C32(0x1BC67793), C32(0x7FB1C723), C32(0xB66FC640),
        C32(0x1A48B71C), C32(0xF0976D17), C32(0x088CE80A), C32(0xA454EDF3),
        C32(0x1C096BF4), C32(0xAC76224B), C32(0x5215781C), C32(0xCD5D2669)
};

static const sph_u32 A_init_224[] = {
        C32(0xA5201467), C32(0xA9B8D94A), C32(0xD4CED997), C32(0x68379D7B),
        C32(0xA7FC73BA), C32(0xF1A2546B), C32(0x606782BF), C32(0xE0BCFD0F),
        C32(0x2F25374E), C32(0x069A149F), C32(0x5E2DFF25), C32(0xFAECF061)
};

static const sph_u32 B_init_224[] = {
        C32(0xEC9905D8), C32(0xF21850CF), C32(0xC0A746C8), C32(0x21DAD498),
        C32(0x35156EEB), C32(0x088C97F2), C32(0x26303E40), C32(0x8A2D4FB5),
        C32(0xFEEE44B6), C32(0x8A1E9573), C32(0x7B81111A), C32(0xCBC139F0),
        C32(0xA3513861), C32(0x1D2C362E), C32(0x918C580E), C32(0xB58E1B9C)
};

static const sph_u32 C_init_224[] = {
        C32(0xE4B573A1), C32(0x4C1A0880), C32(0x1E907C51), C32(0x04807EFD),
        C32(0x3AD8CDE5), C32(0x16B21302), C32(0x02512C53), C32(0x2204CB18),
        C32(0x99405F2D), C32(0xE5B648A1), C32(0x70AB1D43), C32(0xA10C25C2),
        C32(0x16F1AC05), C32(0x38BBEB56), C32(0x9B01DC60), C32(0xB1096D83)
};

static const sph_u32 A_init_256[] = {
        C32(0x52F84552), C32(0xE54B7999), C32(0x2D8EE3EC), C32(0xB9645191),
        C32(0xE0078B86), C32(0xBB7C44C9), C32(0xD2B5C1CA), C32(0xB0D2EB8C),
        C32(0x14CE5A45), C32(0x22AF50DC), C32(0xEFFDBC6B), C32(0xEB21B74A)
};

static const sph_u32 B_init_256[] = {
        C32(0xB555C6EE), C32(0x3E710596), C32(0xA72A652F), C32(0x9301515F),
        C32(0xDA28C1FA), C32(0x696FD868), C32(0x9CB6BF72), C32(0x0AFE4002),
        C32(0xA6E03615), C32(0x5138C1D4), C32(0xBE216306), C32(0xB38B8890),
        C32(0x3EA8B96B), C32(0x3299ACE4), C32(0x30924DD4), C32(0x55CB34A5)
};

static const sph_u32 C_init_256[] = {
        C32(0xB405F031), C32(0xC4233EBA), C32(0xB3733979), C32(0xC0DD9D55),
        C32(0xC51C28AE), C32(0xA327B8E1), C32(0x56C56167), C32(0xED614433),
        C32(0x88B59D60), C32(0x60E2CEBA), C32(0x758B4B8B), C32(0x83E82A7F),
        C32(0xBC968828), C32(0xE6E00BF7), C32(0xBA839E55), C32(0x9B491C60)
};

static const sph_u32 A_init_384[] = {
        C32(0xC8FCA331), C32(0xE55C504E), C32(0x003EBF26), C32(0xBB6B8D83),
        C32(0x7B0448C1), C32(0x41B82789), C32(0x0A7C9601), C32(0x8D659CFF),
        C32(0xB6E2673E), C32(0xCA54C77B), C32(0x1460FD7E), C32(0x3FCB8F2D)
};

static const sph_u32 B_init_384[] = {
        C32(0x527291FC), C32(0x2A16455F), C32(0x78E627E5), C32(0x944F169F),
        C32(0x1CA6F016), C32(0xA854EA25), C32(0x8DB98ABE), C32(0xF2C62641),
        C32(0x30117DCB), C32(0xCF5C4309), C32(0x93711A25), C32(0xF9F671B8),
        C32(0xB01D2116), C32(0x333F4B89), C32(0xB285D165), C32(0x86829B36)
};

static const sph_u32 C_init_384[] = {
        C32(0xF764B11A), C32(0x76172146), C32(0xCEF6934D), C32(0xC6D28399),
        C32(0xFE095F61), C32(0x5E6018B4), C32(0x5048ECF5), C32(0x51353261),
        C32(0x6E6E36DC), C32(0x63130DAD), C32(0xA9C69BD6), C32(0x1E90EA0C),
        C32(0x7C35073B), C32(0x28D95E6D), C32(0xAA340E0D), C32(0xCB3DEE70)
};

static const sph_u32 A_init_512[] = {
        C32(0x20728DFD), C32(0x46C0BD53), C32(0xE782B699), C32(0x55304632),
        C32(0x71B4EF90), C32(0x0EA9E82C), C32(0xDBB930F1), C32(0xFAD06B8B),
        C32(0xBE0CAE40), C32(0x8BD14410), C32(0x76D2ADAC), C32(0x28ACAB7F)
};

static const sph_u32 B_init_512[] = {
        C32(0xC1099CB7), C32(0x07B385F3), C32(0xE7442C26), C32(0xCC8AD640),
        C32(0xEB6F56C7), C32(0x1EA81AA9), C32(0x73B9D314), C32(0x1DE85D08),
        C32(0x48910A5A), C32(0x893B22DB), C32(0xC5A0DF44), C32(0xBBC4324E),
        C32(0x72D2F240), C32(0x75941D99), C32(0x6D8BDE82), C32(0xA1A7502B)
};

static const sph_u32 C_init_512[] = {
        C32(0xD9BF68D1), C32(0x58BAD750), C32(0x56028CB2), C32(0x8134F359),
        C32(0xB5D469D8), C32(0x941A8CC2), C32(0x418B2A6E), C32(0x04052780),
        C32(0x7F07D787), C32(0x5194358F), C32(0x3C60D665), C32(0xBE97D79A),
        C32(0x950C3434), C32(0xAED9A06D), C32(0x2537DC8D), C32(0x7CDB5969)
};

/* END -- automatically generated code. */

static void
shabal_init(sph_shabal_context *cc, unsigned size)
{
        const sph_u32 *A_init, *B_init, *C_init;
        sph_shabal_context *sc;

        switch (size) {
        case 192:
                A_init = A_init_192;
                B_init = B_init_192;
                C_init = C_init_192;
                break;
        case 224:
                A_init = A_init_224;
                B_init = B_init_224;
                C_init = C_init_224;
                break;
        case 256:
                A_init = A_init_256;
                B_init = B_init_256;
                C_init = C_init_256;
                break;
        case 384:
                A_init = A_init_384;
                B_init = B_init_384;
                C_init = C_init_384;
                break;
        case 512:
                A_init = A_init_512;
                B_init = B_init_512;
                C_init = C_init_512;
                break;
        default:
                return;
        }
        sc = cc;
        memcpy(sc->A, A_init, sizeof sc->A);
        memcpy(sc->B, B_init, sizeof sc->B);
        memcpy(sc->C, C_init, sizeof sc->C);
        sc->Wlow = 1;
        sc->Whigh = 0;
        sc->ptr = 0;
}

static void
shabal_core(sph_shabal_context *cc, const unsigned char *data, size_t len)
{
        sph_shabal_context *sc;
        unsigned char *buf;
        size_t ptr;
        shabal_DECL_STATE

        sc = cc;
        buf = sc->buf;
        ptr = sc->ptr;

        if (len < (sizeof sc->buf) - ptr) {
                memcpy(buf + ptr, data, len);
                ptr += len;
                sc->ptr = ptr;
                return;
        }

        shabal_READ_STATE(sc);
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                ptr += clen;
                data += clen;
                len -= clen;
                if (ptr == sizeof sc->buf) {
                        DECODE_BLOCK;
                        INPUT_BLOCK_ADD;
                        XOR_W;
                        APPLY_P;
                        INPUT_BLOCK_SUB;
                        SWAP_BC;
                        INCR_W;
                        ptr = 0;
                }
        }
        shabal_WRITE_STATE(sc);
        sc->ptr = ptr;
}

static void
shabal_close(sph_shabal_context *cc, unsigned ub, unsigned n, void *dst, unsigned size_words)
{
        sph_shabal_context *sc;
        unsigned char *buf;
        size_t ptr;
        int i;
        unsigned z;
        union {
                unsigned char tmp_out[64];
                sph_u32 dummy;
        } u;
        size_t out_len;
        shabal_DECL_STATE

        sc = cc;
        buf = sc->buf;
        ptr = sc->ptr;
        z = 0x80 >> n;
        buf[ptr] = ((ub & -z) | z) & 0xFF;
        memset(buf + ptr + 1, 0, (sizeof sc->buf) - (ptr + 1));
        shabal_READ_STATE(sc);
        DECODE_BLOCK;
        INPUT_BLOCK_ADD;
        XOR_W;
        APPLY_P;
        for (i = 0; i < 3; i ++) {
                SWAP_BC;
                XOR_W;
                APPLY_P;
        }

        switch (size_words) {
        case 16:
                sph_enc32le_aligned(u.tmp_out +  0, B0);
                sph_enc32le_aligned(u.tmp_out +  4, B1);
                sph_enc32le_aligned(u.tmp_out +  8, B2);
                sph_enc32le_aligned(u.tmp_out + 12, B3);
                /* fall through */
        case 12:
                sph_enc32le_aligned(u.tmp_out + 16, B4);
                sph_enc32le_aligned(u.tmp_out + 20, B5);
                sph_enc32le_aligned(u.tmp_out + 24, B6);
                sph_enc32le_aligned(u.tmp_out + 28, B7);
                /* fall through */
        case 8:
                sph_enc32le_aligned(u.tmp_out + 32, B8);
                /* fall through */
        case 7:
                sph_enc32le_aligned(u.tmp_out + 36, B9);
                /* fall through */
        case 6:
                sph_enc32le_aligned(u.tmp_out + 40, BA);
                sph_enc32le_aligned(u.tmp_out + 44, BB);
                sph_enc32le_aligned(u.tmp_out + 48, BC);
                sph_enc32le_aligned(u.tmp_out + 52, BD);
                sph_enc32le_aligned(u.tmp_out + 56, BE);
                sph_enc32le_aligned(u.tmp_out + 60, BF);
                break;
        default:
                return;
        }
        out_len = size_words << 2;
        memcpy(dst, u.tmp_out + (sizeof u.tmp_out) - out_len, out_len);
        shabal_init(sc, size_words << 5);
}

void
sph_shabal192_init(sph_shabal_context *cc)
{
        shabal_init(cc, 192);
}

void
sph_shabal192(sph_shabal_context *cc, const unsigned char *data, size_t len)
{
        shabal_core(cc, data, len);
}

void
sph_shabal192_close(sph_shabal_context *cc, unsigned char *dst)
{
        shabal_close(cc, 0, 0, dst, 6);
}

void
sph_shabal192_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shabal_close(cc, ub, n, dst, 6);
}

void
sph_shabal224_init(sph_shabal_context *cc)
{
        shabal_init(cc, 224);
}

void
sph_shabal224(sph_shabal_context *cc, const unsigned char *data, size_t len)
{
        shabal_core(cc, data, len);
}

void
sph_shabal224_close(sph_shabal_context *cc, unsigned char *dst)
{
        shabal_close(cc, 0, 0, dst, 7);
}

void
sph_shabal224_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shabal_close(cc, ub, n, dst, 7);
}

void
sph_shabal256_init(sph_shabal_context *cc)
{
        shabal_init(cc, 256);
}

void
sph_shabal256(sph_shabal_context *cc, const unsigned char *data, size_t len)
{
        shabal_core(cc, data, len);
}

void
sph_shabal256_close(sph_shabal_context *cc, unsigned char *dst)
{
        shabal_close(cc, 0, 0, dst, 8);
}

void
sph_shabal256_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shabal_close(cc, ub, n, dst, 8);
}

void
sph_shabal384_init(sph_shabal_context *cc)
{
        shabal_init(cc, 384);
}

void
sph_shabal384(sph_shabal_context *cc, const unsigned char *data, size_t len)
{
        shabal_core(cc, data, len);
}

void
sph_shabal384_close(sph_shabal_context *cc, unsigned char *dst)
{
        shabal_close(cc, 0, 0, dst, 12);
}

void
sph_shabal384_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shabal_close(cc, ub, n, dst, 12);
}

void
sph_shabal512_init(sph_shabal_context *cc)
{
        shabal_init(cc, 512);
}

void
sph_shabal512(sph_shabal_context *cc, const unsigned char *data, size_t len)
{
        shabal_core(cc, data, len);
}

void
sph_shabal512_close(sph_shabal_context *cc, unsigned char *dst)
{
        shabal_close(cc, 0, 0, dst, 16);
}

void
sph_shabal512_addbits_and_close(sph_shabal_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shabal_close(cc, ub, n, dst, 16);
}















#define SPH_SIZE_shavite224   224
#define SPH_SIZE_shavite256   256
#define SPH_SIZE_shavite384   384
#define SPH_SIZE_shavite512   512

typedef struct {
        unsigned char buf[64];    /* first field, for alignment */
        size_t ptr;
        sph_u32 h[8];
        sph_u32 count0, count1;
} sph_shavite_small_context;


typedef struct {
        unsigned char buf[128];    /* first field, for alignment */
        size_t ptr;
        sph_u32 h[16];
        sph_u32 count0, count1, count2, count3;
} sph_shavite_big_context;

void
sph_shavite224_init(sph_shavite_small_context *cc);
void
sph_shavite224(sph_shavite_small_context *cc, const unsigned char *data, size_t len);
void
sph_shavite224_close(sph_shavite_small_context *cc, unsigned char *dst);
void
sph_shavite224_addbits_and_close(sph_shavite_small_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_shavite256_init(sph_shavite_small_context *cc);
void
sph_shavite256(sph_shavite_small_context *cc, const unsigned char *data, size_t len);
void
sph_shavite256_close(sph_shavite_small_context *cc, unsigned char *dst);
void
sph_shavite256_addbits_and_close(sph_shavite_small_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_shavite384_init(sph_shavite_big_context *cc);
void
sph_shavite384(sph_shavite_big_context *cc, const unsigned char *data, size_t len);
void
sph_shavite384_close(sph_shavite_big_context *cc, unsigned char *dst);
void
sph_shavite384_addbits_and_close(sph_shavite_big_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_shavite512_init(sph_shavite_big_context *cc);
void
sph_shavite512(sph_shavite_big_context *cc, const unsigned char *data, size_t len);
void
sph_shavite512_close(sph_shavite_big_context *cc, unsigned char *dst);
void
sph_shavite512_addbits_and_close(sph_shavite_big_context *cc, unsigned ub, unsigned n, unsigned char *dst);










#define C32   SPH_C32
#define AES_BIG_ENDIAN   0

static const sph_u32 shavite_IV224[] = {
        C32(0x6774F31C), C32(0x990AE210), C32(0xC87D4274), C32(0xC9546371),
        C32(0x62B2AEA8), C32(0x4B5801D8), C32(0x1B702860), C32(0x842F3017)
};

static const sph_u32 shavite_IV256[] = {
        C32(0x49BB3E47), C32(0x2674860D), C32(0xA8B392AC), C32(0x021AC4E6),
        C32(0x409283CF), C32(0x620E5D86), C32(0x6D929DCB), C32(0x96CC2A8B)
};

static const sph_u32 shavite_IV384[] = {
        C32(0x83DF1545), C32(0xF9AAEC13), C32(0xF4803CB0), C32(0x11FE1F47),
        C32(0xDA6CD269), C32(0x4F53FCD7), C32(0x950529A2), C32(0x97908147),
        C32(0xB0A4D7AF), C32(0x2B9132BF), C32(0x226E607D), C32(0x3C0F8D7C),
        C32(0x487B3F0F), C32(0x04363E22), C32(0x0155C99C), C32(0xEC2E20D3)
};

static const sph_u32 shavite_IV512[] = {
        C32(0x72FCCDD8), C32(0x79CA4727), C32(0x128A077B), C32(0x40D55AEC),
        C32(0xD1901A06), C32(0x430AE307), C32(0xB29F5CD1), C32(0xDF07FBFC),
        C32(0x8E45D73D), C32(0x681AB538), C32(0xBDE86578), C32(0xDD577E47),
        C32(0xE275EADE), C32(0x502D9FCD), C32(0xB9357178), C32(0x022A4B9A)
};

#define AES_ROUND_NOKEY(x0, x1, x2, x3)   do { \
                sph_u32 t0 = (x0); \
                sph_u32 t1 = (x1); \
                sph_u32 t2 = (x2); \
                sph_u32 t3 = (x3); \
                AES_ROUND_NOKEY_LE(t0, t1, t2, t3, x0, x1, x2, x3); \
        } while (0)


#define KEY_EXPAND_ELT(k0, k1, k2, k3)   do { \
                sph_u32 kt; \
                AES_ROUND_NOKEY(k1, k2, k3, k0); \
                kt = (k0); \
                (k0) = (k1); \
                (k1) = (k2); \
                (k2) = (k3); \
                (k3) = kt; \
        } while (0)


static void
c256(sph_shavite_small_context *sc, const void *msg)
{
        sph_u32 p0, p1, p2, p3, p4, p5, p6, p7;
        sph_u32 x0, x1, x2, x3;
        sph_u32 rk0, rk1, rk2, rk3, rk4, rk5, rk6, rk7;
        sph_u32 rk8, rk9, rkA, rkB, rkC, rkD, rkE, rkF;

        p0 = sc->h[0x0];
        p1 = sc->h[0x1];
        p2 = sc->h[0x2];
        p3 = sc->h[0x3];
        p4 = sc->h[0x4];
        p5 = sc->h[0x5];
        p6 = sc->h[0x6];
        p7 = sc->h[0x7];
        /* round 0 */
        rk0 = sph_dec32le_aligned((const unsigned char *)msg +  0);
        x0 = p4 ^ rk0;
        rk1 = sph_dec32le_aligned((const unsigned char *)msg +  4);
        x1 = p5 ^ rk1;
        rk2 = sph_dec32le_aligned((const unsigned char *)msg +  8);
        x2 = p6 ^ rk2;
        rk3 = sph_dec32le_aligned((const unsigned char *)msg + 12);
        x3 = p7 ^ rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk4 = sph_dec32le_aligned((const unsigned char *)msg + 16);
        x0 ^= rk4;
        rk5 = sph_dec32le_aligned((const unsigned char *)msg + 20);
        x1 ^= rk5;
        rk6 = sph_dec32le_aligned((const unsigned char *)msg + 24);
        x2 ^= rk6;
        rk7 = sph_dec32le_aligned((const unsigned char *)msg + 28);
        x3 ^= rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk8 = sph_dec32le_aligned((const unsigned char *)msg + 32);
        x0 ^= rk8;
        rk9 = sph_dec32le_aligned((const unsigned char *)msg + 36);
        x1 ^= rk9;
        rkA = sph_dec32le_aligned((const unsigned char *)msg + 40);
        x2 ^= rkA;
        rkB = sph_dec32le_aligned((const unsigned char *)msg + 44);
        x3 ^= rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p0 ^= x0;
        p1 ^= x1;
        p2 ^= x2;
        p3 ^= x3;
        /* round 1 */
        rkC = sph_dec32le_aligned((const unsigned char *)msg + 48);
        x0 = p0 ^ rkC;
        rkD = sph_dec32le_aligned((const unsigned char *)msg + 52);
        x1 = p1 ^ rkD;
        rkE = sph_dec32le_aligned((const unsigned char *)msg + 56);
        x2 = p2 ^ rkE;
        rkF = sph_dec32le_aligned((const unsigned char *)msg + 60);
        x3 = p3 ^ rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk0, rk1, rk2, rk3);
        rk0 ^= rkC ^ sc->count0;
        rk1 ^= rkD ^ SPH_T32(~sc->count1);
        rk2 ^= rkE;
        rk3 ^= rkF;
        x0 ^= rk0;
        x1 ^= rk1;
        x2 ^= rk2;
        x3 ^= rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk4, rk5, rk6, rk7);
        rk4 ^= rk0;
        rk5 ^= rk1;
        rk6 ^= rk2;
        rk7 ^= rk3;
        x0 ^= rk4;
        x1 ^= rk5;
        x2 ^= rk6;
        x3 ^= rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p4 ^= x0;
        p5 ^= x1;
        p6 ^= x2;
        p7 ^= x3;
        /* round 2 */
        KEY_EXPAND_ELT(rk8, rk9, rkA, rkB);
        rk8 ^= rk4;
        rk9 ^= rk5;
        rkA ^= rk6;
        rkB ^= rk7;
        x0 = p4 ^ rk8;
        x1 = p5 ^ rk9;
        x2 = p6 ^ rkA;
        x3 = p7 ^ rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rkC, rkD, rkE, rkF);
        rkC ^= rk8;
        rkD ^= rk9;
        rkE ^= rkA;
        rkF ^= rkB;
        x0 ^= rkC;
        x1 ^= rkD;
        x2 ^= rkE;
        x3 ^= rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk0 ^= rkD;
        x0 ^= rk0;
        rk1 ^= rkE;
        x1 ^= rk1;
        rk2 ^= rkF;
        x2 ^= rk2;
        rk3 ^= rk0;
        x3 ^= rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p0 ^= x0;
        p1 ^= x1;
        p2 ^= x2;
        p3 ^= x3;
        /* round 3 */
        rk4 ^= rk1;
        x0 = p0 ^ rk4;
        rk5 ^= rk2;
        x1 = p1 ^ rk5;
        rk6 ^= rk3;
        x2 = p2 ^ rk6;
        rk7 ^= rk4;
        x3 = p3 ^ rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk8 ^= rk5;
        x0 ^= rk8;
        rk9 ^= rk6;
        x1 ^= rk9;
        rkA ^= rk7;
        x2 ^= rkA;
        rkB ^= rk8;
        x3 ^= rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rkC ^= rk9;
        x0 ^= rkC;
        rkD ^= rkA;
        x1 ^= rkD;
        rkE ^= rkB;
        x2 ^= rkE;
        rkF ^= rkC;
        x3 ^= rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p4 ^= x0;
        p5 ^= x1;
        p6 ^= x2;
        p7 ^= x3;
        /* round 4 */
        KEY_EXPAND_ELT(rk0, rk1, rk2, rk3);
        rk0 ^= rkC;
        rk1 ^= rkD;
        rk2 ^= rkE;
        rk3 ^= rkF;
        x0 = p4 ^ rk0;
        x1 = p5 ^ rk1;
        x2 = p6 ^ rk2;
        x3 = p7 ^ rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk4, rk5, rk6, rk7);
        rk4 ^= rk0;
        rk5 ^= rk1;
        rk6 ^= rk2;
        rk7 ^= rk3;
        x0 ^= rk4;
        x1 ^= rk5;
        x2 ^= rk6;
        x3 ^= rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk8, rk9, rkA, rkB);
        rk8 ^= rk4;
        rk9 ^= rk5 ^ sc->count1;
        rkA ^= rk6 ^ SPH_T32(~sc->count0);
        rkB ^= rk7;
        x0 ^= rk8;
        x1 ^= rk9;
        x2 ^= rkA;
        x3 ^= rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p0 ^= x0;
        p1 ^= x1;
        p2 ^= x2;
        p3 ^= x3;
        /* round 5 */
        KEY_EXPAND_ELT(rkC, rkD, rkE, rkF);
        rkC ^= rk8;
        rkD ^= rk9;
        rkE ^= rkA;
        rkF ^= rkB;
        x0 = p0 ^ rkC;
        x1 = p1 ^ rkD;
        x2 = p2 ^ rkE;
        x3 = p3 ^ rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk0 ^= rkD;
        x0 ^= rk0;
        rk1 ^= rkE;
        x1 ^= rk1;
        rk2 ^= rkF;
        x2 ^= rk2;
        rk3 ^= rk0;
        x3 ^= rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk4 ^= rk1;
        x0 ^= rk4;
        rk5 ^= rk2;
        x1 ^= rk5;
        rk6 ^= rk3;
        x2 ^= rk6;
        rk7 ^= rk4;
        x3 ^= rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p4 ^= x0;
        p5 ^= x1;
        p6 ^= x2;
        p7 ^= x3;
        /* round 6 */
        rk8 ^= rk5;
        x0 = p4 ^ rk8;
        rk9 ^= rk6;
        x1 = p5 ^ rk9;
        rkA ^= rk7;
        x2 = p6 ^ rkA;
        rkB ^= rk8;
        x3 = p7 ^ rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rkC ^= rk9;
        x0 ^= rkC;
        rkD ^= rkA;
        x1 ^= rkD;
        rkE ^= rkB;
        x2 ^= rkE;
        rkF ^= rkC;
        x3 ^= rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk0, rk1, rk2, rk3);
        rk0 ^= rkC;
        rk1 ^= rkD;
        rk2 ^= rkE;
        rk3 ^= rkF;
        x0 ^= rk0;
        x1 ^= rk1;
        x2 ^= rk2;
        x3 ^= rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p0 ^= x0;
        p1 ^= x1;
        p2 ^= x2;
        p3 ^= x3;
        /* round 7 */
        KEY_EXPAND_ELT(rk4, rk5, rk6, rk7);
        rk4 ^= rk0;
        rk5 ^= rk1;
        rk6 ^= rk2 ^ sc->count1;
        rk7 ^= rk3 ^ SPH_T32(~sc->count0);
        x0 = p0 ^ rk4;
        x1 = p1 ^ rk5;
        x2 = p2 ^ rk6;
        x3 = p3 ^ rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk8, rk9, rkA, rkB);
        rk8 ^= rk4;
        rk9 ^= rk5;
        rkA ^= rk6;
        rkB ^= rk7;
        x0 ^= rk8;
        x1 ^= rk9;
        x2 ^= rkA;
        x3 ^= rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rkC, rkD, rkE, rkF);
        rkC ^= rk8;
        rkD ^= rk9;
        rkE ^= rkA;
        rkF ^= rkB;
        x0 ^= rkC;
        x1 ^= rkD;
        x2 ^= rkE;
        x3 ^= rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p4 ^= x0;
        p5 ^= x1;
        p6 ^= x2;
        p7 ^= x3;
        /* round 8 */
        rk0 ^= rkD;
        x0 = p4 ^ rk0;
        rk1 ^= rkE;
        x1 = p5 ^ rk1;
        rk2 ^= rkF;
        x2 = p6 ^ rk2;
        rk3 ^= rk0;
        x3 = p7 ^ rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk4 ^= rk1;
        x0 ^= rk4;
        rk5 ^= rk2;
        x1 ^= rk5;
        rk6 ^= rk3;
        x2 ^= rk6;
        rk7 ^= rk4;
        x3 ^= rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk8 ^= rk5;
        x0 ^= rk8;
        rk9 ^= rk6;
        x1 ^= rk9;
        rkA ^= rk7;
        x2 ^= rkA;
        rkB ^= rk8;
        x3 ^= rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p0 ^= x0;
        p1 ^= x1;
        p2 ^= x2;
        p3 ^= x3;
        /* round 9 */
        rkC ^= rk9;
        x0 = p0 ^ rkC;
        rkD ^= rkA;
        x1 = p1 ^ rkD;
        rkE ^= rkB;
        x2 = p2 ^ rkE;
        rkF ^= rkC;
        x3 = p3 ^ rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk0, rk1, rk2, rk3);
        rk0 ^= rkC;
        rk1 ^= rkD;
        rk2 ^= rkE;
        rk3 ^= rkF;
        x0 ^= rk0;
        x1 ^= rk1;
        x2 ^= rk2;
        x3 ^= rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk4, rk5, rk6, rk7);
        rk4 ^= rk0;
        rk5 ^= rk1;
        rk6 ^= rk2;
        rk7 ^= rk3;
        x0 ^= rk4;
        x1 ^= rk5;
        x2 ^= rk6;
        x3 ^= rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p4 ^= x0;
        p5 ^= x1;
        p6 ^= x2;
        p7 ^= x3;
        /* round 10 */
        KEY_EXPAND_ELT(rk8, rk9, rkA, rkB);
        rk8 ^= rk4;
        rk9 ^= rk5;
        rkA ^= rk6;
        rkB ^= rk7;
        x0 = p4 ^ rk8;
        x1 = p5 ^ rk9;
        x2 = p6 ^ rkA;
        x3 = p7 ^ rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rkC, rkD, rkE, rkF);
        rkC ^= rk8 ^ sc->count0;
        rkD ^= rk9;
        rkE ^= rkA;
        rkF ^= rkB ^ SPH_T32(~sc->count1);
        x0 ^= rkC;
        x1 ^= rkD;
        x2 ^= rkE;
        x3 ^= rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk0 ^= rkD;
        x0 ^= rk0;
        rk1 ^= rkE;
        x1 ^= rk1;
        rk2 ^= rkF;
        x2 ^= rk2;
        rk3 ^= rk0;
        x3 ^= rk3;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p0 ^= x0;
        p1 ^= x1;
        p2 ^= x2;
        p3 ^= x3;
        /* round 11 */
        rk4 ^= rk1;
        x0 = p0 ^ rk4;
        rk5 ^= rk2;
        x1 = p1 ^ rk5;
        rk6 ^= rk3;
        x2 = p2 ^ rk6;
        rk7 ^= rk4;
        x3 = p3 ^ rk7;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk8 ^= rk5;
        x0 ^= rk8;
        rk9 ^= rk6;
        x1 ^= rk9;
        rkA ^= rk7;
        x2 ^= rkA;
        rkB ^= rk8;
        x3 ^= rkB;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rkC ^= rk9;
        x0 ^= rkC;
        rkD ^= rkA;
        x1 ^= rkD;
        rkE ^= rkB;
        x2 ^= rkE;
        rkF ^= rkC;
        x3 ^= rkF;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p4 ^= x0;
        p5 ^= x1;
        p6 ^= x2;
        p7 ^= x3;
        sc->h[0x0] ^= p0;
        sc->h[0x1] ^= p1;
        sc->h[0x2] ^= p2;
        sc->h[0x3] ^= p3;
        sc->h[0x4] ^= p4;
        sc->h[0x5] ^= p5;
        sc->h[0x6] ^= p6;
        sc->h[0x7] ^= p7;
}



static void
c512(sph_shavite_big_context *sc, const void *msg)
{
        sph_u32 p0, p1, p2, p3, p4, p5, p6, p7;
        sph_u32 p8, p9, pA, pB, pC, pD, pE, pF;
        sph_u32 x0, x1, x2, x3;
        sph_u32 rk00, rk01, rk02, rk03, rk04, rk05, rk06, rk07;
        sph_u32 rk08, rk09, rk0A, rk0B, rk0C, rk0D, rk0E, rk0F;
        sph_u32 rk10, rk11, rk12, rk13, rk14, rk15, rk16, rk17;
        sph_u32 rk18, rk19, rk1A, rk1B, rk1C, rk1D, rk1E, rk1F;
        int r;

        p0 = sc->h[0x0];
        p1 = sc->h[0x1];
        p2 = sc->h[0x2];
        p3 = sc->h[0x3];
        p4 = sc->h[0x4];
        p5 = sc->h[0x5];
        p6 = sc->h[0x6];
        p7 = sc->h[0x7];
        p8 = sc->h[0x8];
        p9 = sc->h[0x9];
        pA = sc->h[0xA];
        pB = sc->h[0xB];
        pC = sc->h[0xC];
        pD = sc->h[0xD];
        pE = sc->h[0xE];
        pF = sc->h[0xF];
        /* round 0 */
        rk00 = sph_dec32le_aligned((const unsigned char *)msg +   0);
        x0 = p4 ^ rk00;
        rk01 = sph_dec32le_aligned((const unsigned char *)msg +   4);
        x1 = p5 ^ rk01;
        rk02 = sph_dec32le_aligned((const unsigned char *)msg +   8);
        x2 = p6 ^ rk02;
        rk03 = sph_dec32le_aligned((const unsigned char *)msg +  12);
        x3 = p7 ^ rk03;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk04 = sph_dec32le_aligned((const unsigned char *)msg +  16);
        x0 ^= rk04;
        rk05 = sph_dec32le_aligned((const unsigned char *)msg +  20);
        x1 ^= rk05;
        rk06 = sph_dec32le_aligned((const unsigned char *)msg +  24);
        x2 ^= rk06;
        rk07 = sph_dec32le_aligned((const unsigned char *)msg +  28);
        x3 ^= rk07;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk08 = sph_dec32le_aligned((const unsigned char *)msg +  32);
        x0 ^= rk08;
        rk09 = sph_dec32le_aligned((const unsigned char *)msg +  36);
        x1 ^= rk09;
        rk0A = sph_dec32le_aligned((const unsigned char *)msg +  40);
        x2 ^= rk0A;
        rk0B = sph_dec32le_aligned((const unsigned char *)msg +  44);
        x3 ^= rk0B;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk0C = sph_dec32le_aligned((const unsigned char *)msg +  48);
        x0 ^= rk0C;
        rk0D = sph_dec32le_aligned((const unsigned char *)msg +  52);
        x1 ^= rk0D;
        rk0E = sph_dec32le_aligned((const unsigned char *)msg +  56);
        x2 ^= rk0E;
        rk0F = sph_dec32le_aligned((const unsigned char *)msg +  60);
        x3 ^= rk0F;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p0 ^= x0;
        p1 ^= x1;
        p2 ^= x2;
        p3 ^= x3;
        rk10 = sph_dec32le_aligned((const unsigned char *)msg +  64);
        x0 = pC ^ rk10;
        rk11 = sph_dec32le_aligned((const unsigned char *)msg +  68);
        x1 = pD ^ rk11;
        rk12 = sph_dec32le_aligned((const unsigned char *)msg +  72);
        x2 = pE ^ rk12;
        rk13 = sph_dec32le_aligned((const unsigned char *)msg +  76);
        x3 = pF ^ rk13;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk14 = sph_dec32le_aligned((const unsigned char *)msg +  80);
        x0 ^= rk14;
        rk15 = sph_dec32le_aligned((const unsigned char *)msg +  84);
        x1 ^= rk15;
        rk16 = sph_dec32le_aligned((const unsigned char *)msg +  88);
        x2 ^= rk16;
        rk17 = sph_dec32le_aligned((const unsigned char *)msg +  92);
        x3 ^= rk17;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk18 = sph_dec32le_aligned((const unsigned char *)msg +  96);
        x0 ^= rk18;
        rk19 = sph_dec32le_aligned((const unsigned char *)msg + 100);
        x1 ^= rk19;
        rk1A = sph_dec32le_aligned((const unsigned char *)msg + 104);
        x2 ^= rk1A;
        rk1B = sph_dec32le_aligned((const unsigned char *)msg + 108);
        x3 ^= rk1B;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        rk1C = sph_dec32le_aligned((const unsigned char *)msg + 112);
        x0 ^= rk1C;
        rk1D = sph_dec32le_aligned((const unsigned char *)msg + 116);
        x1 ^= rk1D;
        rk1E = sph_dec32le_aligned((const unsigned char *)msg + 120);
        x2 ^= rk1E;
        rk1F = sph_dec32le_aligned((const unsigned char *)msg + 124);
        x3 ^= rk1F;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p8 ^= x0;
        p9 ^= x1;
        pA ^= x2;
        pB ^= x3;

        for (r = 0; r < 3; r ++) {
                /* round 1, 5, 9 */
                KEY_EXPAND_ELT(rk00, rk01, rk02, rk03);
                rk00 ^= rk1C;
                rk01 ^= rk1D;
                rk02 ^= rk1E;
                rk03 ^= rk1F;
                if (r == 0) {
                        rk00 ^= sc->count0;
                        rk01 ^= sc->count1;
                        rk02 ^= sc->count2;
                        rk03 ^= SPH_T32(~sc->count3);
                }
                x0 = p0 ^ rk00;
                x1 = p1 ^ rk01;
                x2 = p2 ^ rk02;
                x3 = p3 ^ rk03;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk04, rk05, rk06, rk07);
                rk04 ^= rk00;
                rk05 ^= rk01;
                rk06 ^= rk02;
                rk07 ^= rk03;
                if (r == 1) {
                        rk04 ^= sc->count3;
                        rk05 ^= sc->count2;
                        rk06 ^= sc->count1;
                        rk07 ^= SPH_T32(~sc->count0);
                }
                x0 ^= rk04;
                x1 ^= rk05;
                x2 ^= rk06;
                x3 ^= rk07;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk08, rk09, rk0A, rk0B);
                rk08 ^= rk04;
                rk09 ^= rk05;
                rk0A ^= rk06;
                rk0B ^= rk07;
                x0 ^= rk08;
                x1 ^= rk09;
                x2 ^= rk0A;
                x3 ^= rk0B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk0C, rk0D, rk0E, rk0F);
                rk0C ^= rk08;
                rk0D ^= rk09;
                rk0E ^= rk0A;
                rk0F ^= rk0B;
                x0 ^= rk0C;
                x1 ^= rk0D;
                x2 ^= rk0E;
                x3 ^= rk0F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                pC ^= x0;
                pD ^= x1;
                pE ^= x2;
                pF ^= x3;
                KEY_EXPAND_ELT(rk10, rk11, rk12, rk13);
                rk10 ^= rk0C;
                rk11 ^= rk0D;
                rk12 ^= rk0E;
                rk13 ^= rk0F;
                x0 = p8 ^ rk10;
                x1 = p9 ^ rk11;
                x2 = pA ^ rk12;
                x3 = pB ^ rk13;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk14, rk15, rk16, rk17);
                rk14 ^= rk10;
                rk15 ^= rk11;
                rk16 ^= rk12;
                rk17 ^= rk13;
                x0 ^= rk14;
                x1 ^= rk15;
                x2 ^= rk16;
                x3 ^= rk17;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk18, rk19, rk1A, rk1B);
                rk18 ^= rk14;
                rk19 ^= rk15;
                rk1A ^= rk16;
                rk1B ^= rk17;
                x0 ^= rk18;
                x1 ^= rk19;
                x2 ^= rk1A;
                x3 ^= rk1B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk1C, rk1D, rk1E, rk1F);
                rk1C ^= rk18;
                rk1D ^= rk19;
                rk1E ^= rk1A;
                rk1F ^= rk1B;
                if (r == 2) {
                        rk1C ^= sc->count2;
                        rk1D ^= sc->count3;
                        rk1E ^= sc->count0;
                        rk1F ^= SPH_T32(~sc->count1);
                }
                x0 ^= rk1C;
                x1 ^= rk1D;
                x2 ^= rk1E;
                x3 ^= rk1F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                p4 ^= x0;
                p5 ^= x1;
                p6 ^= x2;
                p7 ^= x3;
                /* round 2, 6, 10 */
                rk00 ^= rk19;
                x0 = pC ^ rk00;
                rk01 ^= rk1A;
                x1 = pD ^ rk01;
                rk02 ^= rk1B;
                x2 = pE ^ rk02;
                rk03 ^= rk1C;
                x3 = pF ^ rk03;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk04 ^= rk1D;
                x0 ^= rk04;
                rk05 ^= rk1E;
                x1 ^= rk05;
                rk06 ^= rk1F;
                x2 ^= rk06;
                rk07 ^= rk00;
                x3 ^= rk07;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk08 ^= rk01;
                x0 ^= rk08;
                rk09 ^= rk02;
                x1 ^= rk09;
                rk0A ^= rk03;
                x2 ^= rk0A;
                rk0B ^= rk04;
                x3 ^= rk0B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk0C ^= rk05;
                x0 ^= rk0C;
                rk0D ^= rk06;
                x1 ^= rk0D;
                rk0E ^= rk07;
                x2 ^= rk0E;
                rk0F ^= rk08;
                x3 ^= rk0F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                p8 ^= x0;
                p9 ^= x1;
                pA ^= x2;
                pB ^= x3;
                rk10 ^= rk09;
                x0 = p4 ^ rk10;
                rk11 ^= rk0A;
                x1 = p5 ^ rk11;
                rk12 ^= rk0B;
                x2 = p6 ^ rk12;
                rk13 ^= rk0C;
                x3 = p7 ^ rk13;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk14 ^= rk0D;
                x0 ^= rk14;
                rk15 ^= rk0E;
                x1 ^= rk15;
                rk16 ^= rk0F;
                x2 ^= rk16;
                rk17 ^= rk10;
                x3 ^= rk17;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk18 ^= rk11;
                x0 ^= rk18;
                rk19 ^= rk12;
                x1 ^= rk19;
                rk1A ^= rk13;
                x2 ^= rk1A;
                rk1B ^= rk14;
                x3 ^= rk1B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk1C ^= rk15;
                x0 ^= rk1C;
                rk1D ^= rk16;
                x1 ^= rk1D;
                rk1E ^= rk17;
                x2 ^= rk1E;
                rk1F ^= rk18;
                x3 ^= rk1F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                p0 ^= x0;
                p1 ^= x1;
                p2 ^= x2;
                p3 ^= x3;
                /* round 3, 7, 11 */
                KEY_EXPAND_ELT(rk00, rk01, rk02, rk03);
                rk00 ^= rk1C;
                rk01 ^= rk1D;
                rk02 ^= rk1E;
                rk03 ^= rk1F;
                x0 = p8 ^ rk00;
                x1 = p9 ^ rk01;
                x2 = pA ^ rk02;
                x3 = pB ^ rk03;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk04, rk05, rk06, rk07);
                rk04 ^= rk00;
                rk05 ^= rk01;
                rk06 ^= rk02;
                rk07 ^= rk03;
                x0 ^= rk04;
                x1 ^= rk05;
                x2 ^= rk06;
                x3 ^= rk07;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk08, rk09, rk0A, rk0B);
                rk08 ^= rk04;
                rk09 ^= rk05;
                rk0A ^= rk06;
                rk0B ^= rk07;
                x0 ^= rk08;
                x1 ^= rk09;
                x2 ^= rk0A;
                x3 ^= rk0B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk0C, rk0D, rk0E, rk0F);
                rk0C ^= rk08;
                rk0D ^= rk09;
                rk0E ^= rk0A;
                rk0F ^= rk0B;
                x0 ^= rk0C;
                x1 ^= rk0D;
                x2 ^= rk0E;
                x3 ^= rk0F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                p4 ^= x0;
                p5 ^= x1;
                p6 ^= x2;
                p7 ^= x3;
                KEY_EXPAND_ELT(rk10, rk11, rk12, rk13);
                rk10 ^= rk0C;
                rk11 ^= rk0D;
                rk12 ^= rk0E;
                rk13 ^= rk0F;
                x0 = p0 ^ rk10;
                x1 = p1 ^ rk11;
                x2 = p2 ^ rk12;
                x3 = p3 ^ rk13;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk14, rk15, rk16, rk17);
                rk14 ^= rk10;
                rk15 ^= rk11;
                rk16 ^= rk12;
                rk17 ^= rk13;
                x0 ^= rk14;
                x1 ^= rk15;
                x2 ^= rk16;
                x3 ^= rk17;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk18, rk19, rk1A, rk1B);
                rk18 ^= rk14;
                rk19 ^= rk15;
                rk1A ^= rk16;
                rk1B ^= rk17;
                x0 ^= rk18;
                x1 ^= rk19;
                x2 ^= rk1A;
                x3 ^= rk1B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                KEY_EXPAND_ELT(rk1C, rk1D, rk1E, rk1F);
                rk1C ^= rk18;
                rk1D ^= rk19;
                rk1E ^= rk1A;
                rk1F ^= rk1B;
                x0 ^= rk1C;
                x1 ^= rk1D;
                x2 ^= rk1E;
                x3 ^= rk1F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                pC ^= x0;
                pD ^= x1;
                pE ^= x2;
                pF ^= x3;
                /* round 4, 8, 12 */
                rk00 ^= rk19;
                x0 = p4 ^ rk00;
                rk01 ^= rk1A;
                x1 = p5 ^ rk01;
                rk02 ^= rk1B;
                x2 = p6 ^ rk02;
                rk03 ^= rk1C;
                x3 = p7 ^ rk03;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk04 ^= rk1D;
                x0 ^= rk04;
                rk05 ^= rk1E;
                x1 ^= rk05;
                rk06 ^= rk1F;
                x2 ^= rk06;
                rk07 ^= rk00;
                x3 ^= rk07;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk08 ^= rk01;
                x0 ^= rk08;
                rk09 ^= rk02;
                x1 ^= rk09;
                rk0A ^= rk03;
                x2 ^= rk0A;
                rk0B ^= rk04;
                x3 ^= rk0B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk0C ^= rk05;
                x0 ^= rk0C;
                rk0D ^= rk06;
                x1 ^= rk0D;
                rk0E ^= rk07;
                x2 ^= rk0E;
                rk0F ^= rk08;
                x3 ^= rk0F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                p0 ^= x0;
                p1 ^= x1;
                p2 ^= x2;
                p3 ^= x3;
                rk10 ^= rk09;
                x0 = pC ^ rk10;
                rk11 ^= rk0A;
                x1 = pD ^ rk11;
                rk12 ^= rk0B;
                x2 = pE ^ rk12;
                rk13 ^= rk0C;
                x3 = pF ^ rk13;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk14 ^= rk0D;
                x0 ^= rk14;
                rk15 ^= rk0E;
                x1 ^= rk15;
                rk16 ^= rk0F;
                x2 ^= rk16;
                rk17 ^= rk10;
                x3 ^= rk17;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk18 ^= rk11;
                x0 ^= rk18;
                rk19 ^= rk12;
                x1 ^= rk19;
                rk1A ^= rk13;
                x2 ^= rk1A;
                rk1B ^= rk14;
                x3 ^= rk1B;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                rk1C ^= rk15;
                x0 ^= rk1C;
                rk1D ^= rk16;
                x1 ^= rk1D;
                rk1E ^= rk17;
                x2 ^= rk1E;
                rk1F ^= rk18;
                x3 ^= rk1F;
                AES_ROUND_NOKEY(x0, x1, x2, x3);
                p8 ^= x0;
                p9 ^= x1;
                pA ^= x2;
                pB ^= x3;
        }
        /* round 13 */
        KEY_EXPAND_ELT(rk00, rk01, rk02, rk03);
        rk00 ^= rk1C;
        rk01 ^= rk1D;
        rk02 ^= rk1E;
        rk03 ^= rk1F;
        x0 = p0 ^ rk00;
        x1 = p1 ^ rk01;
        x2 = p2 ^ rk02;
        x3 = p3 ^ rk03;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk04, rk05, rk06, rk07);
        rk04 ^= rk00;
        rk05 ^= rk01;
        rk06 ^= rk02;
        rk07 ^= rk03;
        x0 ^= rk04;
        x1 ^= rk05;
        x2 ^= rk06;
        x3 ^= rk07;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk08, rk09, rk0A, rk0B);
        rk08 ^= rk04;
        rk09 ^= rk05;
        rk0A ^= rk06;
        rk0B ^= rk07;
        x0 ^= rk08;
        x1 ^= rk09;
        x2 ^= rk0A;
        x3 ^= rk0B;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk0C, rk0D, rk0E, rk0F);
        rk0C ^= rk08;
        rk0D ^= rk09;
        rk0E ^= rk0A;
        rk0F ^= rk0B;
        x0 ^= rk0C;
        x1 ^= rk0D;
        x2 ^= rk0E;
        x3 ^= rk0F;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        pC ^= x0;
        pD ^= x1;
        pE ^= x2;
        pF ^= x3;
        KEY_EXPAND_ELT(rk10, rk11, rk12, rk13);
        rk10 ^= rk0C;
        rk11 ^= rk0D;
        rk12 ^= rk0E;
        rk13 ^= rk0F;
        x0 = p8 ^ rk10;
        x1 = p9 ^ rk11;
        x2 = pA ^ rk12;
        x3 = pB ^ rk13;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk14, rk15, rk16, rk17);
        rk14 ^= rk10;
        rk15 ^= rk11;
        rk16 ^= rk12;
        rk17 ^= rk13;
        x0 ^= rk14;
        x1 ^= rk15;
        x2 ^= rk16;
        x3 ^= rk17;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk18, rk19, rk1A, rk1B);
        rk18 ^= rk14 ^ sc->count1;
        rk19 ^= rk15 ^ sc->count0;
        rk1A ^= rk16 ^ sc->count3;
        rk1B ^= rk17 ^ SPH_T32(~sc->count2);
        x0 ^= rk18;
        x1 ^= rk19;
        x2 ^= rk1A;
        x3 ^= rk1B;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        KEY_EXPAND_ELT(rk1C, rk1D, rk1E, rk1F);
        rk1C ^= rk18;
        rk1D ^= rk19;
        rk1E ^= rk1A;
        rk1F ^= rk1B;
        x0 ^= rk1C;
        x1 ^= rk1D;
        x2 ^= rk1E;
        x3 ^= rk1F;
        AES_ROUND_NOKEY(x0, x1, x2, x3);
        p4 ^= x0;
        p5 ^= x1;
        p6 ^= x2;
        p7 ^= x3;
        sc->h[0x0] ^= p8;
        sc->h[0x1] ^= p9;
        sc->h[0x2] ^= pA;
        sc->h[0x3] ^= pB;
        sc->h[0x4] ^= pC;
        sc->h[0x5] ^= pD;
        sc->h[0x6] ^= pE;
        sc->h[0x7] ^= pF;
        sc->h[0x8] ^= p0;
        sc->h[0x9] ^= p1;
        sc->h[0xA] ^= p2;
        sc->h[0xB] ^= p3;
        sc->h[0xC] ^= p4;
        sc->h[0xD] ^= p5;
        sc->h[0xE] ^= p6;
        sc->h[0xF] ^= p7;
}


static void
shavite_small_init(sph_shavite_small_context *sc, const sph_u32 *iv)
{
        memcpy(sc->h, iv, sizeof sc->h);
        sc->ptr = 0;
        sc->count0 = 0;
        sc->count1 = 0;
}

static void
shavite_small_core(sph_shavite_small_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;

        buf = sc->buf;
        ptr = sc->ptr;
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                data = (const unsigned char *)data + clen;
                ptr += clen;
                len -= clen;
                if (ptr == sizeof sc->buf) {
                        if ((sc->count0 = SPH_T32(sc->count0 + 512)) == 0)
                                sc->count1 = SPH_T32(sc->count1 + 1);
                        c256(sc, buf);
                        ptr = 0;
                }
        }
        sc->ptr = ptr;
}

static void
shavite_small_close(sph_shavite_small_context *sc,
        unsigned ub, unsigned n, void *dst, size_t out_size_w32)
{
        unsigned char *buf;
        size_t ptr, u;
        unsigned z;
        sph_u32 count0, count1;

        buf = sc->buf;
        ptr = sc->ptr;
        count0 = (sc->count0 += (ptr << 3) + n);
        count1 = sc->count1;
        z = 0x80 >> n;
        z = ((ub & -z) | z) & 0xFF;
        if (ptr == 0 && n == 0) {
                buf[0] = 0x80;
                memset(buf + 1, 0, 53);
                sc->count0 = sc->count1 = 0;
        } else if (ptr < 54) {
                buf[ptr ++] = z;
                memset(buf + ptr, 0, 54 - ptr);
        } else {
                buf[ptr ++] = z;
                memset(buf + ptr, 0, 64 - ptr);
                c256(sc, buf);
                memset(buf, 0, 54);
                sc->count0 = sc->count1 = 0;
        }
        sph_enc32le(buf + 54, count0);
        sph_enc32le(buf + 58, count1);
        buf[62] = out_size_w32 << 5;
        buf[63] = out_size_w32 >> 3;
        c256(sc, buf);
        for (u = 0; u < out_size_w32; u ++)
                sph_enc32le((unsigned char *)dst + (u << 2), sc->h[u]);
}

static void
shavite_big_init(sph_shavite_big_context *sc, const sph_u32 *iv)
{
        memcpy(sc->h, iv, sizeof sc->h);
        sc->ptr = 0;
        sc->count0 = 0;
        sc->count1 = 0;
        sc->count2 = 0;
        sc->count3 = 0;
}

static void
shavite_big_core(sph_shavite_big_context *sc, const void *data, size_t len)
{
        unsigned char *buf;
        size_t ptr;

        buf = sc->buf;
        ptr = sc->ptr;
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - ptr;
                if (clen > len)
                        clen = len;
                memcpy(buf + ptr, data, clen);
                data = (const unsigned char *)data + clen;
                ptr += clen;
                len -= clen;
                if (ptr == sizeof sc->buf) {
                        if ((sc->count0 = SPH_T32(sc->count0 + 1024)) == 0) {
                                sc->count1 = SPH_T32(sc->count1 + 1);
                                if (sc->count1 == 0) {
                                        sc->count2 = SPH_T32(sc->count2 + 1);
                                        if (sc->count2 == 0) {
                                                sc->count3 = SPH_T32(
                                                        sc->count3 + 1);
                                        }
                                }
                        }
                        c512(sc, buf);
                        ptr = 0;
                }
        }
        sc->ptr = ptr;
}

static void
shavite_big_close(sph_shavite_big_context *sc,
        unsigned ub, unsigned n, void *dst, size_t out_size_w32)
{
        unsigned char *buf;
        size_t ptr, u;
        unsigned z;
        sph_u32 count0, count1, count2, count3;

        buf = sc->buf;
        ptr = sc->ptr;
        count0 = (sc->count0 += (ptr << 3) + n);
        count1 = sc->count1;
        count2 = sc->count2;
        count3 = sc->count3;
        z = 0x80 >> n;
        z = ((ub & -z) | z) & 0xFF;
        if (ptr == 0 && n == 0) {
                buf[0] = 0x80;
                memset(buf + 1, 0, 109);
                sc->count0 = sc->count1 = sc->count2 = sc->count3 = 0;
        } else if (ptr < 110) {
                buf[ptr ++] = z;
                memset(buf + ptr, 0, 110 - ptr);
        } else {
                buf[ptr ++] = z;
                memset(buf + ptr, 0, 128 - ptr);
                c512(sc, buf);
                memset(buf, 0, 110);
                sc->count0 = sc->count1 = sc->count2 = sc->count3 = 0;
        }
        sph_enc32le(buf + 110, count0);
        sph_enc32le(buf + 114, count1);
        sph_enc32le(buf + 118, count2);
        sph_enc32le(buf + 122, count3);
        buf[126] = out_size_w32 << 5;
        buf[127] = out_size_w32 >> 3;
        c512(sc, buf);
        for (u = 0; u < out_size_w32; u ++)
                sph_enc32le((unsigned char *)dst + (u << 2), sc->h[u]);
}

void
sph_shavite224_init(sph_shavite_small_context *cc)
{
        shavite_small_init(cc, shavite_IV224);
}

void
sph_shavite224(sph_shavite_small_context *cc, const unsigned char *data, size_t len)
{
        shavite_small_core(cc, data, len);
}

void
sph_shavite224_close(sph_shavite_small_context *cc, unsigned char *dst)
{
        shavite_small_close(cc, 0, 0, dst, 7);
        shavite_small_init(cc, shavite_IV224);
}

void
sph_shavite224_addbits_and_close(sph_shavite_small_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shavite_small_close(cc, ub, n, dst, 7);
        shavite_small_init(cc, shavite_IV224);
}

void
sph_shavite256_init(sph_shavite_small_context *cc)
{
        shavite_small_init(cc, shavite_IV256);
}

void
sph_shavite256(sph_shavite_small_context *cc, const unsigned char *data, size_t len)
{
        shavite_small_core(cc, data, len);
}

void
sph_shavite256_close(sph_shavite_small_context *cc, unsigned char *dst)
{
        shavite_small_close(cc, 0, 0, dst, 8);
        shavite_small_init(cc, shavite_IV256);
}

void
sph_shavite256_addbits_and_close(sph_shavite_small_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shavite_small_close(cc, ub, n, dst, 8);
        shavite_small_init(cc, shavite_IV256);
}

void
sph_shavite384_init(sph_shavite_big_context *cc)
{
        shavite_big_init(cc, shavite_IV384);
}

void
sph_shavite384(sph_shavite_big_context *cc, const unsigned char *data, size_t len)
{
        shavite_big_core(cc, data, len);
}

void
sph_shavite384_close(sph_shavite_big_context *cc, unsigned char *dst)
{
        shavite_big_close(cc, 0, 0, dst, 12);
        shavite_big_init(cc, shavite_IV384);
}

void
sph_shavite384_addbits_and_close(sph_shavite_big_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shavite_big_close(cc, ub, n, dst, 12);
        shavite_big_init(cc, shavite_IV384);
}

void
sph_shavite512_init(sph_shavite_big_context *cc)
{
        shavite_big_init(cc, shavite_IV512);
}

void
sph_shavite512(sph_shavite_big_context *cc, const unsigned char *data, size_t len)
{
        shavite_big_core(cc, data, len);
}

void
sph_shavite512_close(sph_shavite_big_context *cc, unsigned char *dst)
{
        shavite_big_close(cc, 0, 0, dst, 16);
        shavite_big_init(cc, shavite_IV512);
}

void
sph_shavite512_addbits_and_close(sph_shavite_big_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        shavite_big_close(cc, ub, n, dst, 16);
        shavite_big_init(cc, shavite_IV512);
}
















#define SPH_SIZE_simd224   224
#define SPH_SIZE_simd256   256
#define SPH_SIZE_simd384   384
#define SPH_SIZE_simd512   512

typedef struct {
        unsigned char buf[64];    /* first field, for alignment */
        size_t ptr;
        sph_u32 state[16];
        sph_u32 count_low, count_high;
} sph_simd_small_context;


typedef struct {
        unsigned char buf[128];    /* first field, for alignment */
        size_t ptr;
        sph_u32 state[32];
        sph_u32 count_low, count_high;
} sph_simd_big_context;

void
sph_simd224_init(sph_simd_small_context *cc);
void
sph_simd224(sph_simd_small_context *cc, const unsigned char *data, size_t len);
void
sph_simd224_close(sph_simd_small_context *cc, unsigned char *dst);
void
sph_simd224_addbits_and_close(sph_simd_small_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_simd256_init(sph_simd_small_context *cc);
void
sph_simd256(sph_simd_small_context *cc, const unsigned char *data, size_t len);
void
sph_simd256_close(sph_simd_small_context *cc, unsigned char *dst);
void
sph_simd256_addbits_and_close(sph_simd_small_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_simd384_init(sph_simd_big_context *cc);
void
sph_simd384(sph_simd_big_context *cc, const unsigned char *data, size_t len);
void
sph_simd384_close(sph_simd_big_context *cc, unsigned char *dst);
void
sph_simd384_addbits_and_close(sph_simd_big_context *cc, unsigned ub, unsigned n, unsigned char *dst);
void
sph_simd512_init(sph_simd_big_context *cc);
void
sph_simd512(sph_simd_big_context *cc, const unsigned char *data, size_t len);
void
sph_simd512_close(sph_simd_big_context *cc, unsigned char *dst);
void
sph_simd512_addbits_and_close(sph_simd_big_context *cc, unsigned ub, unsigned n, unsigned char *dst);









typedef sph_u32 u32;
typedef sph_s32 s32;
#define C32     SPH_C32
#define T32     SPH_T32
#define ROL32   SPH_ROTL32

#define XCAT(x, y)    XCAT_(x, y)
#define XCAT_(x, y)   x ## y

static const s32 alpha_tab[] = {
          1,  41, 139,  45,  46,  87, 226,  14,  60, 147, 116, 130,
        190,  80, 196,  69,   2,  82,  21,  90,  92, 174, 195,  28,
        120,  37, 232,   3, 123, 160, 135, 138,   4, 164,  42, 180,
        184,  91, 133,  56, 240,  74, 207,   6, 246,  63,  13,  19,
          8,  71,  84, 103, 111, 182,   9, 112, 223, 148, 157,  12,
        235, 126,  26,  38,  16, 142, 168, 206, 222, 107,  18, 224,
        189,  39,  57,  24, 213, 252,  52,  76,  32,  27,  79, 155,
        187, 214,  36, 191, 121,  78, 114,  48, 169, 247, 104, 152,
         64,  54, 158,  53, 117, 171,  72, 125, 242, 156, 228,  96,
         81, 237, 208,  47, 128, 108,  59, 106, 234,  85, 144, 250,
        227,  55, 199, 192, 162, 217, 159,  94, 256, 216, 118, 212,
        211, 170,  31, 243, 197, 110, 141, 127,  67, 177,  61, 188,
        255, 175, 236, 167, 165,  83,  62, 229, 137, 220,  25, 254,
        134,  97, 122, 119, 253,  93, 215,  77,  73, 166, 124, 201,
         17, 183,  50, 251,  11, 194, 244, 238, 249, 186, 173, 154,
        146,  75, 248, 145,  34, 109, 100, 245,  22, 131, 231, 219,
        241, 115,  89,  51,  35, 150, 239,  33,  68, 218, 200, 233,
         44,   5, 205, 181, 225, 230, 178, 102,  70,  43, 221,  66,
        136, 179, 143, 209,  88,  10, 153, 105, 193, 203,  99, 204,
        140,  86, 185, 132,  15, 101,  29, 161, 176,  20,  49, 210,
        129, 149, 198, 151,  23, 172, 113,   7,  30, 202,  58,  65,
         95,  40,  98, 163
};

#define REDS1(x)    (((x) & 0xFF) - ((x) >> 8))
#define REDS2(x)    (((x) & 0xFFFF) + ((x) >> 16))

#define FFT_LOOP(rb, hk, as, id)   do { \
                size_t u, v; \
                s32 m = q[(rb)]; \
                s32 n = q[(rb) + (hk)]; \
                q[(rb)] = m + n; \
                q[(rb) + (hk)] = m - n; \
                u = v = 0; \
                goto id; \
                for (; u < (hk); u += 4, v += 4 * (as)) { \
                        s32 t; \
                        m = q[(rb) + u + 0]; \
                        n = q[(rb) + u + 0 + (hk)]; \
                        t = REDS2(n * alpha_tab[v + 0 * (as)]); \
                        q[(rb) + u + 0] = m + t; \
                        q[(rb) + u + 0 + (hk)] = m - t; \
                id: \
                        m = q[(rb) + u + 1]; \
                        n = q[(rb) + u + 1 + (hk)]; \
                        t = REDS2(n * alpha_tab[v + 1 * (as)]); \
                        q[(rb) + u + 1] = m + t; \
                        q[(rb) + u + 1 + (hk)] = m - t; \
                        m = q[(rb) + u + 2]; \
                        n = q[(rb) + u + 2 + (hk)]; \
                        t = REDS2(n * alpha_tab[v + 2 * (as)]); \
                        q[(rb) + u + 2] = m + t; \
                        q[(rb) + u + 2 + (hk)] = m - t; \
                        m = q[(rb) + u + 3]; \
                        n = q[(rb) + u + 3 + (hk)]; \
                        t = REDS2(n * alpha_tab[v + 3 * (as)]); \
                        q[(rb) + u + 3] = m + t; \
                        q[(rb) + u + 3 + (hk)] = m - t; \
                } \
        } while (0)

#define FFT8(xb, xs, d)   do { \
                s32 x0 = x[(xb)]; \
                s32 x1 = x[(xb) + (xs)]; \
                s32 x2 = x[(xb) + 2 * (xs)]; \
                s32 x3 = x[(xb) + 3 * (xs)]; \
                s32 a0 = x0 + x2; \
                s32 a1 = x0 + (x2 << 4); \
                s32 a2 = x0 - x2; \
                s32 a3 = x0 - (x2 << 4); \
                s32 b0 = x1 + x3; \
                s32 b1 = REDS1((x1 << 2) + (x3 << 6)); \
                s32 b2 = (x1 << 4) - (x3 << 4); \
                s32 b3 = REDS1((x1 << 6) + (x3 << 2)); \
                d ## 0 = a0 + b0; \
                d ## 1 = a1 + b1; \
                d ## 2 = a2 + b2; \
                d ## 3 = a3 + b3; \
                d ## 4 = a0 - b0; \
                d ## 5 = a1 - b1; \
                d ## 6 = a2 - b2; \
                d ## 7 = a3 - b3; \
        } while (0)

#define FFT16(xb, xs, rb)   do { \
                s32 d1_0, d1_1, d1_2, d1_3, d1_4, d1_5, d1_6, d1_7; \
                s32 d2_0, d2_1, d2_2, d2_3, d2_4, d2_5, d2_6, d2_7; \
                FFT8(xb, (xs) << 1, d1_); \
                FFT8((xb) + (xs), (xs) << 1, d2_); \
                q[(rb) +  0] = d1_0 + d2_0; \
                q[(rb) +  1] = d1_1 + (d2_1 << 1); \
                q[(rb) +  2] = d1_2 + (d2_2 << 2); \
                q[(rb) +  3] = d1_3 + (d2_3 << 3); \
                q[(rb) +  4] = d1_4 + (d2_4 << 4); \
                q[(rb) +  5] = d1_5 + (d2_5 << 5); \
                q[(rb) +  6] = d1_6 + (d2_6 << 6); \
                q[(rb) +  7] = d1_7 + (d2_7 << 7); \
                q[(rb) +  8] = d1_0 - d2_0; \
                q[(rb) +  9] = d1_1 - (d2_1 << 1); \
                q[(rb) + 10] = d1_2 - (d2_2 << 2); \
                q[(rb) + 11] = d1_3 - (d2_3 << 3); \
                q[(rb) + 12] = d1_4 - (d2_4 << 4); \
                q[(rb) + 13] = d1_5 - (d2_5 << 5); \
                q[(rb) + 14] = d1_6 - (d2_6 << 6); \
                q[(rb) + 15] = d1_7 - (d2_7 << 7); \
        } while (0)

#define FFT32(xb, xs, rb, id)   do { \
                FFT16(xb, (xs) << 1, rb); \
                FFT16((xb) + (xs), (xs) << 1, (rb) + 16); \
                FFT_LOOP(rb, 16, 8, id); \
        } while (0)

#define FFT64(xb, xs, rb, id)   do { \
                FFT32(xb, (xs) << 1, rb, XCAT(id, a)); \
                FFT32((xb) + (xs), (xs) << 1, (rb) + 32, XCAT(id, b)); \
                FFT_LOOP(rb, 32, 4, id); \
        } while (0)


#define FFT128(xb, xs, rb, id)   do { \
                FFT64(xb, (xs) << 1, rb, XCAT(id, a)); \
                FFT64((xb) + (xs), (xs) << 1, (rb) + 64, XCAT(id, b)); \
                FFT_LOOP(rb, 64, 2, id); \
        } while (0)



static void
fft64(unsigned char *x, size_t xs, s32 *q)
{
        size_t xd;

        xd = xs << 1;
        FFT32(0, xd, 0, label_a);
        FFT32(xs, xd, 32, label_b);
        FFT_LOOP(0, 32, 4, label_);
}

#define FFT256(xb, xs, rb, id)   do { \
                fft64(x + (xb) + ((xs) * 0), (xs) << 2, &q[(rb) +   0]); \
                fft64(x + (xb) + ((xs) * 2), (xs) << 2, &q[(rb) +  64]); \
                FFT_LOOP(rb, 64, 2, XCAT(id, aa)); \
                fft64(x + (xb) + ((xs) * 1), (xs) << 2, &q[(rb) + 128]); \
                fft64(x + (xb) + ((xs) * 3), (xs) << 2, &q[(rb) + 192]); \
                FFT_LOOP((rb) + 128, 64, 2, XCAT(id, ab)); \
                FFT_LOOP(rb, 128, 1, XCAT(id, a)); \
        } while (0)

static const unsigned short yoff_s_n[] = {
          1,  98,  95,  58,  30, 113,  23, 198, 129,  49, 176,  29,
         15, 185, 140,  99, 193, 153,  88, 143, 136, 221,  70, 178,
        225, 205,  44, 200,  68, 239,  35,  89, 241, 231,  22, 100,
         34, 248, 146, 173, 249, 244,  11,  50,  17, 124,  73, 215,
        253, 122, 134,  25, 137,  62, 165, 236, 255,  61,  67, 141,
        197,  31, 211, 118, 256, 159, 162, 199, 227, 144, 234,  59,
        128, 208,  81, 228, 242,  72, 117, 158,  64, 104, 169, 114,
        121,  36, 187,  79,  32,  52, 213,  57, 189,  18, 222, 168,
         16,  26, 235, 157, 223,   9, 111,  84,   8,  13, 246, 207,
        240, 133, 184,  42,   4, 135, 123, 232, 120, 195,  92,  21,
          2, 196, 190, 116,  60, 226,  46, 139
};

static const unsigned short yoff_s_f[] = {
          2, 156, 118, 107,  45, 212, 111, 162,  97, 249, 211,   3,
         49, 101, 151, 223, 189, 178, 253, 204,  76,  82, 232,  65,
         96, 176, 161,  47, 189,  61, 248, 107,   0, 131, 133, 113,
         17,  33,  12, 111, 251, 103,  57, 148,  47,  65, 249, 143,
        189,   8, 204, 230, 205, 151, 187, 227, 247, 111, 140,   6,
         77,  10,  21, 149, 255, 101, 139, 150, 212,  45, 146,  95,
        160,   8,  46, 254, 208, 156, 106,  34,  68,  79,   4,  53,
        181, 175,  25, 192, 161,  81,  96, 210,  68, 196,   9, 150,
          0, 126, 124, 144, 240, 224, 245, 146,   6, 154, 200, 109,
        210, 192,   8, 114,  68, 249,  53,  27,  52, 106,  70,  30,
         10, 146, 117, 251, 180, 247, 236, 108
};

static const unsigned short yoff_b_n[] = {
          1, 163,  98,  40,  95,  65,  58, 202,  30,   7, 113, 172,
         23, 151, 198, 149, 129, 210,  49,  20, 176, 161,  29, 101,
         15, 132, 185,  86, 140, 204,  99, 203, 193, 105, 153,  10,
         88, 209, 143, 179, 136,  66, 221,  43,  70, 102, 178, 230,
        225, 181, 205,   5,  44, 233, 200, 218,  68,  33, 239, 150,
         35,  51,  89, 115, 241, 219, 231, 131,  22, 245, 100, 109,
         34, 145, 248,  75, 146, 154, 173, 186, 249, 238, 244, 194,
         11, 251,  50, 183,  17, 201, 124, 166,  73,  77, 215,  93,
        253, 119, 122,  97, 134, 254,  25, 220, 137, 229,  62,  83,
        165, 167, 236, 175, 255, 188,  61, 177,  67, 127, 141, 110,
        197, 243,  31, 170, 211, 212, 118, 216, 256,  94, 159, 217,
        162, 192, 199,  55, 227, 250, 144,  85, 234, 106,  59, 108,
        128,  47, 208, 237,  81,  96, 228, 156, 242, 125,  72, 171,
        117,  53, 158,  54,  64, 152, 104, 247, 169,  48, 114,  78,
        121, 191,  36, 214, 187, 155,  79,  27,  32,  76,  52, 252,
        213,  24,  57,  39, 189, 224,  18, 107, 222, 206, 168, 142,
         16,  38,  26, 126, 235,  12, 157, 148, 223, 112,   9, 182,
        111, 103,  84,  71,   8,  19,  13,  63, 246,   6, 207,  74,
        240,  56, 133,  91, 184, 180,  42, 164,   4, 138, 135, 160,
        123,   3, 232,  37, 120,  28, 195, 174,  92,  90,  21,  82,
          2,  69, 196,  80, 190, 130, 116, 147,  60,  14, 226,  87,
         46,  45, 139,  41
};

static const unsigned short yoff_b_f[] = {
          2, 203, 156,  47, 118, 214, 107, 106,  45,  93, 212,  20,
        111,  73, 162, 251,  97, 215, 249,  53, 211,  19,   3,  89,
         49, 207, 101,  67, 151, 130, 223,  23, 189, 202, 178, 239,
        253, 127, 204,  49,  76, 236,  82, 137, 232, 157,  65,  79,
         96, 161, 176, 130, 161,  30,  47,   9, 189, 247,  61, 226,
        248,  90, 107,  64,   0,  88, 131, 243, 133,  59, 113, 115,
         17, 236,  33, 213,  12, 191, 111,  19, 251,  61, 103, 208,
         57,  35, 148, 248,  47, 116,  65, 119, 249, 178, 143,  40,
        189, 129,   8, 163, 204, 227, 230, 196, 205, 122, 151,  45,
        187,  19, 227,  72, 247, 125, 111, 121, 140, 220,   6, 107,
         77,  69,  10, 101,  21,  65, 149, 171, 255,  54, 101, 210,
        139,  43, 150, 151, 212, 164,  45, 237, 146, 184,  95,   6,
        160,  42,   8, 204,  46, 238, 254, 168, 208,  50, 156, 190,
        106, 127,  34, 234,  68,  55,  79,  18,   4, 130,  53, 208,
        181,  21, 175, 120,  25, 100, 192, 178, 161,  96,  81, 127,
         96, 227, 210, 248,  68,  10, 196,  31,   9, 167, 150, 193,
          0, 169, 126,  14, 124, 198, 144, 142, 240,  21, 224,  44,
        245,  66, 146, 238,   6, 196, 154,  49, 200, 222, 109,   9,
        210, 141, 192, 138,   8,  79, 114, 217,  68, 128, 249,  94,
         53,  30,  27,  61,  52, 135, 106, 212,  70, 238,  30, 185,
         10, 132, 146, 136, 117,  37, 251, 150, 180, 188, 247, 156,
        236, 192, 108,  86
};

#define INNER(l, h, mm)   (((u32)((l) * (mm)) & 0xFFFFU) \
                          + ((u32)((h) * (mm)) << 16))

#define W_SMALL(sb, o1, o2, mm) \
        (INNER(q[8 * (sb) + 2 * 0 + o1], q[8 * (sb) + 2 * 0 + o2], mm), \
         INNER(q[8 * (sb) + 2 * 1 + o1], q[8 * (sb) + 2 * 1 + o2], mm), \
         INNER(q[8 * (sb) + 2 * 2 + o1], q[8 * (sb) + 2 * 2 + o2], mm), \
         INNER(q[8 * (sb) + 2 * 3 + o1], q[8 * (sb) + 2 * 3 + o2], mm)

#define WS_0_0   W_SMALL( 4,    0,    1, 185)
#define WS_0_1   W_SMALL( 6,    0,    1, 185)
#define WS_0_2   W_SMALL( 0,    0,    1, 185)
#define WS_0_3   W_SMALL( 2,    0,    1, 185)
#define WS_0_4   W_SMALL( 7,    0,    1, 185)
#define WS_0_5   W_SMALL( 5,    0,    1, 185)
#define WS_0_6   W_SMALL( 3,    0,    1, 185)
#define WS_0_7   W_SMALL( 1,    0,    1, 185)
#define WS_1_0   W_SMALL(15,    0,    1, 185)
#define WS_1_1   W_SMALL(11,    0,    1, 185)
#define WS_1_2   W_SMALL(12,    0,    1, 185)
#define WS_1_3   W_SMALL( 8,    0,    1, 185)
#define WS_1_4   W_SMALL( 9,    0,    1, 185)
#define WS_1_5   W_SMALL(13,    0,    1, 185)
#define WS_1_6   W_SMALL(10,    0,    1, 185)
#define WS_1_7   W_SMALL(14,    0,    1, 185)
#define WS_2_0   W_SMALL(17, -128,  -64, 233)
#define WS_2_1   W_SMALL(18, -128,  -64, 233)
#define WS_2_2   W_SMALL(23, -128,  -64, 233)
#define WS_2_3   W_SMALL(20, -128,  -64, 233)
#define WS_2_4   W_SMALL(22, -128,  -64, 233)
#define WS_2_5   W_SMALL(21, -128,  -64, 233)
#define WS_2_6   W_SMALL(16, -128,  -64, 233)
#define WS_2_7   W_SMALL(19, -128,  -64, 233)
#define WS_3_0   W_SMALL(30, -191, -127, 233)
#define WS_3_1   W_SMALL(24, -191, -127, 233)
#define WS_3_2   W_SMALL(25, -191, -127, 233)
#define WS_3_3   W_SMALL(31, -191, -127, 233)
#define WS_3_4   W_SMALL(27, -191, -127, 233)
#define WS_3_5   W_SMALL(29, -191, -127, 233)
#define WS_3_6   W_SMALL(28, -191, -127, 233)
#define WS_3_7   W_SMALL(26, -191, -127, 233)

#define W_BIG(sb, o1, o2, mm) \
        (INNER(q[16 * (sb) + 2 * 0 + o1], q[16 * (sb) + 2 * 0 + o2], mm), \
         INNER(q[16 * (sb) + 2 * 1 + o1], q[16 * (sb) + 2 * 1 + o2], mm), \
         INNER(q[16 * (sb) + 2 * 2 + o1], q[16 * (sb) + 2 * 2 + o2], mm), \
         INNER(q[16 * (sb) + 2 * 3 + o1], q[16 * (sb) + 2 * 3 + o2], mm), \
         INNER(q[16 * (sb) + 2 * 4 + o1], q[16 * (sb) + 2 * 4 + o2], mm), \
         INNER(q[16 * (sb) + 2 * 5 + o1], q[16 * (sb) + 2 * 5 + o2], mm), \
         INNER(q[16 * (sb) + 2 * 6 + o1], q[16 * (sb) + 2 * 6 + o2], mm), \
         INNER(q[16 * (sb) + 2 * 7 + o1], q[16 * (sb) + 2 * 7 + o2], mm)

#define WB_0_0   W_BIG( 4,    0,    1, 185)
#define WB_0_1   W_BIG( 6,    0,    1, 185)
#define WB_0_2   W_BIG( 0,    0,    1, 185)
#define WB_0_3   W_BIG( 2,    0,    1, 185)
#define WB_0_4   W_BIG( 7,    0,    1, 185)
#define WB_0_5   W_BIG( 5,    0,    1, 185)
#define WB_0_6   W_BIG( 3,    0,    1, 185)
#define WB_0_7   W_BIG( 1,    0,    1, 185)
#define WB_1_0   W_BIG(15,    0,    1, 185)
#define WB_1_1   W_BIG(11,    0,    1, 185)
#define WB_1_2   W_BIG(12,    0,    1, 185)
#define WB_1_3   W_BIG( 8,    0,    1, 185)
#define WB_1_4   W_BIG( 9,    0,    1, 185)
#define WB_1_5   W_BIG(13,    0,    1, 185)
#define WB_1_6   W_BIG(10,    0,    1, 185)
#define WB_1_7   W_BIG(14,    0,    1, 185)
#define WB_2_0   W_BIG(17, -256, -128, 233)
#define WB_2_1   W_BIG(18, -256, -128, 233)
#define WB_2_2   W_BIG(23, -256, -128, 233)
#define WB_2_3   W_BIG(20, -256, -128, 233)
#define WB_2_4   W_BIG(22, -256, -128, 233)
#define WB_2_5   W_BIG(21, -256, -128, 233)
#define WB_2_6   W_BIG(16, -256, -128, 233)
#define WB_2_7   W_BIG(19, -256, -128, 233)
#define WB_3_0   W_BIG(30, -383, -255, 233)
#define WB_3_1   W_BIG(24, -383, -255, 233)
#define WB_3_2   W_BIG(25, -383, -255, 233)
#define WB_3_3   W_BIG(31, -383, -255, 233)
#define WB_3_4   W_BIG(27, -383, -255, 233)
#define WB_3_5   W_BIG(29, -383, -255, 233)
#define WB_3_6   W_BIG(28, -383, -255, 233)
#define WB_3_7   W_BIG(26, -383, -255, 233)

#define IF(x, y, z)    ((((y) ^ (z)) & (x)) ^ (z))
#define MAJ(x, y, z)   (((x) & (y)) | (((x) | (y)) & (z)))

#define PP4_0_0   1
#define PP4_0_1   0
#define PP4_0_2   3
#define PP4_0_3   2
#define PP4_1_0   2
#define PP4_1_1   3
#define PP4_1_2   0
#define PP4_1_3   1
#define PP4_2_0   3
#define PP4_2_1   2
#define PP4_2_2   1
#define PP4_2_3   0

#define PP8_0_0   1
#define PP8_0_1   0
#define PP8_0_2   3
#define PP8_0_3   2
#define PP8_0_4   5
#define PP8_0_5   4
#define PP8_0_6   7
#define PP8_0_7   6

#define PP8_1_0   6
#define PP8_1_1   7
#define PP8_1_2   4
#define PP8_1_3   5
#define PP8_1_4   2
#define PP8_1_5   3
#define PP8_1_6   0
#define PP8_1_7   1

#define PP8_2_0   2
#define PP8_2_1   3
#define PP8_2_2   0
#define PP8_2_3   1
#define PP8_2_4   6
#define PP8_2_5   7
#define PP8_2_6   4
#define PP8_2_7   5

#define PP8_3_0   3
#define PP8_3_1   2
#define PP8_3_2   1
#define PP8_3_3   0
#define PP8_3_4   7
#define PP8_3_5   6
#define PP8_3_6   5
#define PP8_3_7   4

#define PP8_4_0   5
#define PP8_4_1   4
#define PP8_4_2   7
#define PP8_4_3   6
#define PP8_4_4   1
#define PP8_4_5   0
#define PP8_4_6   3
#define PP8_4_7   2

#define PP8_5_0   7
#define PP8_5_1   6
#define PP8_5_2   5
#define PP8_5_3   4
#define PP8_5_4   3
#define PP8_5_5   2
#define PP8_5_6   1
#define PP8_5_7   0

#define PP8_6_0   4
#define PP8_6_1   5
#define PP8_6_2   6
#define PP8_6_3   7
#define PP8_6_4   0
#define PP8_6_5   1
#define PP8_6_6   2
#define PP8_6_7   3


#define simd_DECL_STATE_SMALL   \
        u32 A0, A1, A2, A3, B0, B1, B2, B3, C0, C1, C2, C3, D0, D1, D2, D3;

#define simd_READ_STATE_SMALL(sc)   do { \
                A0 = (sc)->state[ 0]; \
                A1 = (sc)->state[ 1]; \
                A2 = (sc)->state[ 2]; \
                A3 = (sc)->state[ 3]; \
                B0 = (sc)->state[ 4]; \
                B1 = (sc)->state[ 5]; \
                B2 = (sc)->state[ 6]; \
                B3 = (sc)->state[ 7]; \
                C0 = (sc)->state[ 8]; \
                C1 = (sc)->state[ 9]; \
                C2 = (sc)->state[10]; \
                C3 = (sc)->state[11]; \
                D0 = (sc)->state[12]; \
                D1 = (sc)->state[13]; \
                D2 = (sc)->state[14]; \
                D3 = (sc)->state[15]; \
        } while (0)

#define simd_WRITE_STATE_SMALL(sc)   do { \
                (sc)->state[ 0] = A0; \
                (sc)->state[ 1] = A1; \
                (sc)->state[ 2] = A2; \
                (sc)->state[ 3] = A3; \
                (sc)->state[ 4] = B0; \
                (sc)->state[ 5] = B1; \
                (sc)->state[ 6] = B2; \
                (sc)->state[ 7] = B3; \
                (sc)->state[ 8] = C0; \
                (sc)->state[ 9] = C1; \
                (sc)->state[10] = C2; \
                (sc)->state[11] = C3; \
                (sc)->state[12] = D0; \
                (sc)->state[13] = D1; \
                (sc)->state[14] = D2; \
                (sc)->state[15] = D3; \
        } while (0)

#define simd_DECL_STATE_BIG   \
        u32 A0, A1, A2, A3, A4, A5, A6, A7; \
        u32 B0, B1, B2, B3, B4, B5, B6, B7; \
        u32 C0, C1, C2, C3, C4, C5, C6, C7; \
        u32 D0, D1, D2, D3, D4, D5, D6, D7;

#define simd_READ_STATE_BIG(sc)   do { \
                A0 = (sc)->state[ 0]; \
                A1 = (sc)->state[ 1]; \
                A2 = (sc)->state[ 2]; \
                A3 = (sc)->state[ 3]; \
                A4 = (sc)->state[ 4]; \
                A5 = (sc)->state[ 5]; \
                A6 = (sc)->state[ 6]; \
                A7 = (sc)->state[ 7]; \
                B0 = (sc)->state[ 8]; \
                B1 = (sc)->state[ 9]; \
                B2 = (sc)->state[10]; \
                B3 = (sc)->state[11]; \
                B4 = (sc)->state[12]; \
                B5 = (sc)->state[13]; \
                B6 = (sc)->state[14]; \
                B7 = (sc)->state[15]; \
                C0 = (sc)->state[16]; \
                C1 = (sc)->state[17]; \
                C2 = (sc)->state[18]; \
                C3 = (sc)->state[19]; \
                C4 = (sc)->state[20]; \
                C5 = (sc)->state[21]; \
                C6 = (sc)->state[22]; \
                C7 = (sc)->state[23]; \
                D0 = (sc)->state[24]; \
                D1 = (sc)->state[25]; \
                D2 = (sc)->state[26]; \
                D3 = (sc)->state[27]; \
                D4 = (sc)->state[28]; \
                D5 = (sc)->state[29]; \
                D6 = (sc)->state[30]; \
                D7 = (sc)->state[31]; \
        } while (0)

#define simd_WRITE_STATE_BIG(sc)   do { \
                (sc)->state[ 0] = A0; \
                (sc)->state[ 1] = A1; \
                (sc)->state[ 2] = A2; \
                (sc)->state[ 3] = A3; \
                (sc)->state[ 4] = A4; \
                (sc)->state[ 5] = A5; \
                (sc)->state[ 6] = A6; \
                (sc)->state[ 7] = A7; \
                (sc)->state[ 8] = B0; \
                (sc)->state[ 9] = B1; \
                (sc)->state[10] = B2; \
                (sc)->state[11] = B3; \
                (sc)->state[12] = B4; \
                (sc)->state[13] = B5; \
                (sc)->state[14] = B6; \
                (sc)->state[15] = B7; \
                (sc)->state[16] = C0; \
                (sc)->state[17] = C1; \
                (sc)->state[18] = C2; \
                (sc)->state[19] = C3; \
                (sc)->state[20] = C4; \
                (sc)->state[21] = C5; \
                (sc)->state[22] = C6; \
                (sc)->state[23] = C7; \
                (sc)->state[24] = D0; \
                (sc)->state[25] = D1; \
                (sc)->state[26] = D2; \
                (sc)->state[27] = D3; \
                (sc)->state[28] = D4; \
                (sc)->state[29] = D5; \
                (sc)->state[30] = D6; \
                (sc)->state[31] = D7; \
        } while (0)


#define STEP_ELT(n, w, fun, s, ppb)   do { \
                u32 tt = T32(D ## n + (w) + fun(A ## n, B ## n, C ## n)); \
                A ## n = T32(ROL32(tt, s) + XCAT(tA, XCAT(ppb, n))); \
                D ## n = C ## n; \
                C ## n = B ## n; \
                B ## n = tA ## n; \
        } while (0)

#define STEP_SMALL(w0, w1, w2, w3, fun, r, s, pp4b)   do { \
                u32 tA0 = ROL32(A0, r); \
                u32 tA1 = ROL32(A1, r); \
                u32 tA2 = ROL32(A2, r); \
                u32 tA3 = ROL32(A3, r); \
                STEP_ELT(0, w0, fun, s, pp4b); \
                STEP_ELT(1, w1, fun, s, pp4b); \
                STEP_ELT(2, w2, fun, s, pp4b); \
                STEP_ELT(3, w3, fun, s, pp4b); \
        } while (0)

#define STEP_BIG(w0, w1, w2, w3, w4, w5, w6, w7, fun, r, s, pp8b)   do { \
                u32 tA0 = ROL32(A0, r); \
                u32 tA1 = ROL32(A1, r); \
                u32 tA2 = ROL32(A2, r); \
                u32 tA3 = ROL32(A3, r); \
                u32 tA4 = ROL32(A4, r); \
                u32 tA5 = ROL32(A5, r); \
                u32 tA6 = ROL32(A6, r); \
                u32 tA7 = ROL32(A7, r); \
                STEP_ELT(0, w0, fun, s, pp8b); \
                STEP_ELT(1, w1, fun, s, pp8b); \
                STEP_ELT(2, w2, fun, s, pp8b); \
                STEP_ELT(3, w3, fun, s, pp8b); \
                STEP_ELT(4, w4, fun, s, pp8b); \
                STEP_ELT(5, w5, fun, s, pp8b); \
                STEP_ELT(6, w6, fun, s, pp8b); \
                STEP_ELT(7, w7, fun, s, pp8b); \
        } while (0)

#define M3_0_0   0_
#define M3_1_0   1_
#define M3_2_0   2_
#define M3_3_0   0_
#define M3_4_0   1_
#define M3_5_0   2_
#define M3_6_0   0_
#define M3_7_0   1_

#define M3_0_1   1_
#define M3_1_1   2_
#define M3_2_1   0_
#define M3_3_1   1_
#define M3_4_1   2_
#define M3_5_1   0_
#define M3_6_1   1_
#define M3_7_1   2_

#define M3_0_2   2_
#define M3_1_2   0_
#define M3_2_2   1_
#define M3_3_2   2_
#define M3_4_2   0_
#define M3_5_2   1_
#define M3_6_2   2_
#define M3_7_2   0_

#define STEP_SMALL_(w, fun, r, s, pp4b)   STEP_SMALL w, fun, r, s, pp4b)

#define ONE_ROUND_SMALL(ri, isp, p0, p1, p2, p3)   do { \
                STEP_SMALL_(WS_ ## ri ## 0, \
                        IF,  p0, p1, XCAT(PP4_, M3_0_ ## isp)); \
                STEP_SMALL_(WS_ ## ri ## 1, \
                        IF,  p1, p2, XCAT(PP4_, M3_1_ ## isp)); \
                STEP_SMALL_(WS_ ## ri ## 2, \
                        IF,  p2, p3, XCAT(PP4_, M3_2_ ## isp)); \
                STEP_SMALL_(WS_ ## ri ## 3, \
                        IF,  p3, p0, XCAT(PP4_, M3_3_ ## isp)); \
                STEP_SMALL_(WS_ ## ri ## 4, \
                        MAJ, p0, p1, XCAT(PP4_, M3_4_ ## isp)); \
                STEP_SMALL_(WS_ ## ri ## 5, \
                        MAJ, p1, p2, XCAT(PP4_, M3_5_ ## isp)); \
                STEP_SMALL_(WS_ ## ri ## 6, \
                        MAJ, p2, p3, XCAT(PP4_, M3_6_ ## isp)); \
                STEP_SMALL_(WS_ ## ri ## 7, \
                        MAJ, p3, p0, XCAT(PP4_, M3_7_ ## isp)); \
        } while (0)

#define M7_0_0   0_
#define M7_1_0   1_
#define M7_2_0   2_
#define M7_3_0   3_
#define M7_4_0   4_
#define M7_5_0   5_
#define M7_6_0   6_
#define M7_7_0   0_

#define M7_0_1   1_
#define M7_1_1   2_
#define M7_2_1   3_
#define M7_3_1   4_
#define M7_4_1   5_
#define M7_5_1   6_
#define M7_6_1   0_
#define M7_7_1   1_

#define M7_0_2   2_
#define M7_1_2   3_
#define M7_2_2   4_
#define M7_3_2   5_
#define M7_4_2   6_
#define M7_5_2   0_
#define M7_6_2   1_
#define M7_7_2   2_

#define M7_0_3   3_
#define M7_1_3   4_
#define M7_2_3   5_
#define M7_3_3   6_
#define M7_4_3   0_
#define M7_5_3   1_
#define M7_6_3   2_
#define M7_7_3   3_

#define STEP_BIG_(w, fun, r, s, pp8b)   STEP_BIG w, fun, r, s, pp8b)

#define ONE_ROUND_BIG(ri, isp, p0, p1, p2, p3)   do { \
                STEP_BIG_(WB_ ## ri ## 0, \
                        IF,  p0, p1, XCAT(PP8_, M7_0_ ## isp)); \
                STEP_BIG_(WB_ ## ri ## 1, \
                        IF,  p1, p2, XCAT(PP8_, M7_1_ ## isp)); \
                STEP_BIG_(WB_ ## ri ## 2, \
                        IF,  p2, p3, XCAT(PP8_, M7_2_ ## isp)); \
                STEP_BIG_(WB_ ## ri ## 3, \
                        IF,  p3, p0, XCAT(PP8_, M7_3_ ## isp)); \
                STEP_BIG_(WB_ ## ri ## 4, \
                        MAJ, p0, p1, XCAT(PP8_, M7_4_ ## isp)); \
                STEP_BIG_(WB_ ## ri ## 5, \
                        MAJ, p1, p2, XCAT(PP8_, M7_5_ ## isp)); \
                STEP_BIG_(WB_ ## ri ## 6, \
                        MAJ, p2, p3, XCAT(PP8_, M7_6_ ## isp)); \
                STEP_BIG_(WB_ ## ri ## 7, \
                        MAJ, p3, p0, XCAT(PP8_, M7_7_ ## isp)); \
        } while (0)


static void
compress_small(sph_simd_small_context *sc, int last)
{
        unsigned char *x;
        s32 q[128];
        int i;
        simd_DECL_STATE_SMALL

        x = sc->buf;
        FFT128(0, 1, 0, ll);
        if (last) {
                for (i = 0; i < 128; i ++) {
                        s32 tq;

                        tq = q[i] + yoff_s_f[i];
                        tq = REDS2(tq);
                        tq = REDS1(tq);
                        tq = REDS1(tq);
                        q[i] = (tq <= 128 ? tq : tq - 257);
                }
        } else {
                for (i = 0; i < 128; i ++) {
                        s32 tq;

                        tq = q[i] + yoff_s_n[i];
                        tq = REDS2(tq);
                        tq = REDS1(tq);
                        tq = REDS1(tq);
                        q[i] = (tq <= 128 ? tq : tq - 257);
                }
        }
        simd_READ_STATE_SMALL(sc);
        A0 ^= sph_dec32le_aligned(x +  0);
        A1 ^= sph_dec32le_aligned(x +  4);
        A2 ^= sph_dec32le_aligned(x +  8);
        A3 ^= sph_dec32le_aligned(x + 12);
        B0 ^= sph_dec32le_aligned(x + 16);
        B1 ^= sph_dec32le_aligned(x + 20);
        B2 ^= sph_dec32le_aligned(x + 24);
        B3 ^= sph_dec32le_aligned(x + 28);
        C0 ^= sph_dec32le_aligned(x + 32);
        C1 ^= sph_dec32le_aligned(x + 36);
        C2 ^= sph_dec32le_aligned(x + 40);
        C3 ^= sph_dec32le_aligned(x + 44);
        D0 ^= sph_dec32le_aligned(x + 48);
        D1 ^= sph_dec32le_aligned(x + 52);
        D2 ^= sph_dec32le_aligned(x + 56);
        D3 ^= sph_dec32le_aligned(x + 60);
        ONE_ROUND_SMALL(0_, 0,  3, 23, 17, 27);
        ONE_ROUND_SMALL(1_, 2, 28, 19, 22,  7);
        ONE_ROUND_SMALL(2_, 1, 29,  9, 15,  5);
        ONE_ROUND_SMALL(3_, 0,  4, 13, 10, 25);
        STEP_SMALL(sc->state[ 0], sc->state[ 1], sc->state[ 2], sc->state[ 3],
                IF,  4, 13, PP4_2_);
        STEP_SMALL(sc->state[ 4], sc->state[ 5], sc->state[ 6], sc->state[ 7],
                IF, 13, 10, PP4_0_);
        STEP_SMALL(sc->state[ 8], sc->state[ 9], sc->state[10], sc->state[11],
                IF, 10, 25, PP4_1_);
        STEP_SMALL(sc->state[12], sc->state[13], sc->state[14], sc->state[15],
                IF, 25,  4, PP4_2_);
        simd_WRITE_STATE_SMALL(sc);
}





static void
compress_big(sph_simd_big_context *sc, int last)
{
        unsigned char *x;
        s32 q[256];
        int i;
        simd_DECL_STATE_BIG

        x = sc->buf;
        FFT256(0, 1, 0, ll);
        if (last) {
                for (i = 0; i < 256; i ++) {
                        s32 tq;

                        tq = q[i] + yoff_b_f[i];
                        tq = REDS2(tq);
                        tq = REDS1(tq);
                        tq = REDS1(tq);
                        q[i] = (tq <= 128 ? tq : tq - 257);
                }
        } else {
                for (i = 0; i < 256; i ++) {
                        s32 tq;

                        tq = q[i] + yoff_b_n[i];
                        tq = REDS2(tq);
                        tq = REDS1(tq);
                        tq = REDS1(tq);
                        q[i] = (tq <= 128 ? tq : tq - 257);
                }
        }
        simd_READ_STATE_BIG(sc);
        A0 ^= sph_dec32le_aligned(x +   0);
        A1 ^= sph_dec32le_aligned(x +   4);
        A2 ^= sph_dec32le_aligned(x +   8);
        A3 ^= sph_dec32le_aligned(x +  12);
        A4 ^= sph_dec32le_aligned(x +  16);
        A5 ^= sph_dec32le_aligned(x +  20);
        A6 ^= sph_dec32le_aligned(x +  24);
        A7 ^= sph_dec32le_aligned(x +  28);
        B0 ^= sph_dec32le_aligned(x +  32);
        B1 ^= sph_dec32le_aligned(x +  36);
        B2 ^= sph_dec32le_aligned(x +  40);
        B3 ^= sph_dec32le_aligned(x +  44);
        B4 ^= sph_dec32le_aligned(x +  48);
        B5 ^= sph_dec32le_aligned(x +  52);
        B6 ^= sph_dec32le_aligned(x +  56);
        B7 ^= sph_dec32le_aligned(x +  60);
        C0 ^= sph_dec32le_aligned(x +  64);
        C1 ^= sph_dec32le_aligned(x +  68);
        C2 ^= sph_dec32le_aligned(x +  72);
        C3 ^= sph_dec32le_aligned(x +  76);
        C4 ^= sph_dec32le_aligned(x +  80);
        C5 ^= sph_dec32le_aligned(x +  84);
        C6 ^= sph_dec32le_aligned(x +  88);
        C7 ^= sph_dec32le_aligned(x +  92);
        D0 ^= sph_dec32le_aligned(x +  96);
        D1 ^= sph_dec32le_aligned(x + 100);
        D2 ^= sph_dec32le_aligned(x + 104);
        D3 ^= sph_dec32le_aligned(x + 108);
        D4 ^= sph_dec32le_aligned(x + 112);
        D5 ^= sph_dec32le_aligned(x + 116);
        D6 ^= sph_dec32le_aligned(x + 120);
        D7 ^= sph_dec32le_aligned(x + 124);

        ONE_ROUND_BIG(0_, 0,  3, 23, 17, 27);
        ONE_ROUND_BIG(1_, 1, 28, 19, 22,  7);
        ONE_ROUND_BIG(2_, 2, 29,  9, 15,  5);
        ONE_ROUND_BIG(3_, 3,  4, 13, 10, 25);
        STEP_BIG(
                sc->state[ 0], sc->state[ 1], sc->state[ 2], sc->state[ 3],
                sc->state[ 4], sc->state[ 5], sc->state[ 6], sc->state[ 7],
                IF,  4, 13, PP8_4_);
        STEP_BIG(
                sc->state[ 8], sc->state[ 9], sc->state[10], sc->state[11],
                sc->state[12], sc->state[13], sc->state[14], sc->state[15],
                IF, 13, 10, PP8_5_);
        STEP_BIG(
                sc->state[16], sc->state[17], sc->state[18], sc->state[19],
                sc->state[20], sc->state[21], sc->state[22], sc->state[23],
                IF, 10, 25, PP8_6_);
        STEP_BIG(
                sc->state[24], sc->state[25], sc->state[26], sc->state[27],
                sc->state[28], sc->state[29], sc->state[30], sc->state[31],
                IF, 25,  4, PP8_0_);
        simd_WRITE_STATE_BIG(sc);
}


static const u32 simd_IV224[] = {
        C32(0x33586E9F), C32(0x12FFF033), C32(0xB2D9F64D), C32(0x6F8FEA53),
        C32(0xDE943106), C32(0x2742E439), C32(0x4FBAB5AC), C32(0x62B9FF96),
        C32(0x22E7B0AF), C32(0xC862B3A8), C32(0x33E00CDC), C32(0x236B86A6),
        C32(0xF64AE77C), C32(0xFA373B76), C32(0x7DC1EE5B), C32(0x7FB29CE8)
};

static const u32 simd_IV256[] = {
        C32(0x4D567983), C32(0x07190BA9), C32(0x8474577B), C32(0x39D726E9),
        C32(0xAAF3D925), C32(0x3EE20B03), C32(0xAFD5E751), C32(0xC96006D3),
        C32(0xC2C2BA14), C32(0x49B3BCB4), C32(0xF67CAF46), C32(0x668626C9),
        C32(0xE2EAA8D2), C32(0x1FF47833), C32(0xD0C661A5), C32(0x55693DE1)
};

static const u32 simd_IV384[] = {
        C32(0x8A36EEBC), C32(0x94A3BD90), C32(0xD1537B83), C32(0xB25B070B),
        C32(0xF463F1B5), C32(0xB6F81E20), C32(0x0055C339), C32(0xB4D144D1),
        C32(0x7360CA61), C32(0x18361A03), C32(0x17DCB4B9), C32(0x3414C45A),
        C32(0xA699A9D2), C32(0xE39E9664), C32(0x468BFE77), C32(0x51D062F8),
        C32(0xB9E3BFE8), C32(0x63BECE2A), C32(0x8FE506B9), C32(0xF8CC4AC2),
        C32(0x7AE11542), C32(0xB1AADDA1), C32(0x64B06794), C32(0x28D2F462),
        C32(0xE64071EC), C32(0x1DEB91A8), C32(0x8AC8DB23), C32(0x3F782AB5),
        C32(0x039B5CB8), C32(0x71DDD962), C32(0xFADE2CEA), C32(0x1416DF71)
};

static const u32 simd_IV512[] = {
        C32(0x0BA16B95), C32(0x72F999AD), C32(0x9FECC2AE), C32(0xBA3264FC),
        C32(0x5E894929), C32(0x8E9F30E5), C32(0x2F1DAA37), C32(0xF0F2C558),
        C32(0xAC506643), C32(0xA90635A5), C32(0xE25B878B), C32(0xAAB7878F),
        C32(0x88817F7A), C32(0x0A02892B), C32(0x559A7550), C32(0x598F657E),
        C32(0x7EEF60A1), C32(0x6B70E3E8), C32(0x9C1714D1), C32(0xB958E2A8),
        C32(0xAB02675E), C32(0xED1C014F), C32(0xCD8D65BB), C32(0xFDB7A257),
        C32(0x09254899), C32(0xD699C7BC), C32(0x9019B6DC), C32(0x2B9022E4),
        C32(0x8FA14956), C32(0x21BF9BD3), C32(0xB94D0943), C32(0x6FFDDC22)
};

static void
init_small(sph_simd_small_context *cc, const u32 *iv)
{
        sph_simd_small_context *sc;

        sc = cc;
        memcpy(sc->state, iv, sizeof sc->state);
        sc->count_low = sc->count_high = 0;
        sc->ptr = 0;
}

static void
init_big(sph_simd_big_context *cc, const u32 *iv)
{
        sph_simd_big_context *sc;

        sc = cc;
        memcpy(sc->state, iv, sizeof sc->state);
        sc->count_low = sc->count_high = 0;
        sc->ptr = 0;
}

static void
update_small(sph_simd_small_context *cc, const void *data, size_t len)
{
        sph_simd_small_context *sc;

        sc = cc;
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - sc->ptr;
                if (clen > len)
                        clen = len;
                memcpy(sc->buf + sc->ptr, data, clen);
                data = (const unsigned char *)data + clen;
                len -= clen;
                if ((sc->ptr += clen) == sizeof sc->buf) {
                        compress_small(sc, 0);
                        sc->ptr = 0;
                        sc->count_low = T32(sc->count_low + 1);
                        if (sc->count_low == 0)
                                sc->count_high ++;
                }
        }
}

static void
update_big(sph_simd_big_context *cc, const void *data, size_t len)
{
        sph_simd_big_context *sc;

        sc = cc;
        while (len > 0) {
                size_t clen;

                clen = (sizeof sc->buf) - sc->ptr;
                if (clen > len)
                        clen = len;
                memcpy(sc->buf + sc->ptr, data, clen);
                data = (const unsigned char *)data + clen;
                len -= clen;
                if ((sc->ptr += clen) == sizeof sc->buf) {
                        compress_big(sc, 0);
                        sc->ptr = 0;
                        sc->count_low = T32(sc->count_low + 1);
                        if (sc->count_low == 0)
                                sc->count_high ++;
                }
        }
}

static void
encode_count_small(unsigned char *dst,
        u32 low, u32 high, size_t ptr, unsigned n)
{
        low = T32(low << 9);
        high = T32(high << 9) + (low >> 23);
        low += (ptr << 3) + n;
        sph_enc32le(dst, low);
        sph_enc32le(dst + 4, high);
}

static void
encode_count_big(unsigned char *dst,
        u32 low, u32 high, size_t ptr, unsigned n)
{
        low = T32(low << 10);
        high = T32(high << 10) + (low >> 22);
        low += (ptr << 3) + n;
        sph_enc32le(dst, low);
        sph_enc32le(dst + 4, high);
}

static void
finalize_small(sph_simd_small_context *cc, unsigned ub, unsigned n, unsigned char *dst, size_t dst_len)
{
        sph_simd_small_context *sc;
        unsigned char *d;
        size_t u;

        sc = cc;
        if (sc->ptr > 0 || n > 0) {
                memset(sc->buf + sc->ptr, 0,
                        (sizeof sc->buf) - sc->ptr);
                sc->buf[sc->ptr] = ub & (0xFF << (8 - n));
                compress_small(sc, 0);
        }
        memset(sc->buf, 0, sizeof sc->buf);
        encode_count_small(sc->buf, sc->count_low, sc->count_high, sc->ptr, n);
        compress_small(sc, 1);
        d = dst;
        for (d = dst, u = 0; u < dst_len; u ++)
                sph_enc32le(d + (u << 2), sc->state[u]);
}

static void
finalize_big(sph_simd_big_context *cc, unsigned ub, unsigned n, unsigned char *dst, size_t dst_len)
{
        sph_simd_big_context *sc;
        unsigned char *d;
        size_t u;

        sc = cc;
        if (sc->ptr > 0 || n > 0) {
                memset(sc->buf + sc->ptr, 0,
                        (sizeof sc->buf) - sc->ptr);
                sc->buf[sc->ptr] = ub & (0xFF << (8 - n));
                compress_big(sc, 0);
        }
        memset(sc->buf, 0, sizeof sc->buf);
        encode_count_big(sc->buf, sc->count_low, sc->count_high, sc->ptr, n);
        compress_big(sc, 1);
        d = dst;
        for (d = dst, u = 0; u < dst_len; u ++)
                sph_enc32le(d + (u << 2), sc->state[u]);
}

void
sph_simd224_init(sph_simd_small_context *cc)
{
        init_small(cc, simd_IV224);
}

void
sph_simd224(sph_simd_small_context *cc, const unsigned char *data, size_t len)
{
        update_small(cc, data, len);
}

void
sph_simd224_close(sph_simd_small_context *cc, unsigned char *dst)
{
        sph_simd224_addbits_and_close(cc, 0, 0, dst);
}

void
sph_simd224_addbits_and_close(sph_simd_small_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        finalize_small(cc, ub, n, dst, 7);
        sph_simd224_init(cc);
}

void
sph_simd256_init(sph_simd_small_context *cc)
{
        init_small(cc, simd_IV256);
}

void
sph_simd256(sph_simd_small_context *cc, const unsigned char *data, size_t len)
{
        update_small(cc, data, len);
}

void
sph_simd256_close(sph_simd_small_context *cc, unsigned char *dst)
{
        sph_simd256_addbits_and_close(cc, 0, 0, dst);
}

void
sph_simd256_addbits_and_close(sph_simd_small_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        finalize_small(cc, ub, n, dst, 8);
        sph_simd256_init(cc);
}

void
sph_simd384_init(sph_simd_big_context *cc)
{
        init_big(cc, simd_IV384);
}

void
sph_simd384(sph_simd_big_context *cc, const unsigned char *data, size_t len)
{
        update_big(cc, data, len);
}

void
sph_simd384_close(sph_simd_big_context *cc, unsigned char *dst)
{
        sph_simd384_addbits_and_close(cc, 0, 0, dst);
}

void
sph_simd384_addbits_and_close(sph_simd_big_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        finalize_big(cc, ub, n, dst, 12);
        sph_simd384_init(cc);
}

void
sph_simd512_init(sph_simd_big_context *cc)
{
        init_big(cc, simd_IV512);
}

void
sph_simd512(sph_simd_big_context *cc, const unsigned char *data, size_t len)
{
        update_big(cc, data, len);
}

void
sph_simd512_close(sph_simd_big_context *cc, unsigned char *dst)
{
        sph_simd512_addbits_and_close(cc, 0, 0, dst);
}

void
sph_simd512_addbits_and_close(sph_simd_big_context *cc, unsigned ub, unsigned n, unsigned char *dst)
{
        finalize_big(cc, ub, n, dst, 16);
        sph_simd512_init(cc);
}
