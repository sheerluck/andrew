/*
This code is written by kerukuro and released into public domain.
*/

#include <string>
#include <array>
#include <algorithm>
#include <vector>
#include <iterator>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <cstddef> // needed for testing std::byte
#include <cstdint> // needed for uint64_t

namespace digestpp
{
  //////////detail//////////
  namespace detail
  {
    template <typename T>
    struct is_xof
    {
      static const bool value = T::is_xof;
    };

    template <typename T>
    struct is_byte
    {
      static const bool value = std::is_same<T, char>::value ||
                       std::is_same<T,   signed char>::value ||
                       std::is_same<T,     std::byte>::value ||
                       std::is_same<T, unsigned char>::value;
    };

    template <typename T, size_t N>
    struct stream_width_fixer
    {
      stream_width_fixer(T value)
      : v{value}
      {}

      T v;
    };

    template <typename T, size_t N>
    std::ostream& operator<<(std::ostream& ostr,
                             const stream_width_fixer<T, N>& value)
    {
      return ostr << std::setw(N) << value.v;
    }

    // Byte-swap a 16-bit unsigned integer.
    inline std::uint16_t byteswap(std::uint16_t val)
    {
      return ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
    }

    // Byte-swap a 32-bit unsigned integer.
    inline std::uint32_t byteswap(std::uint32_t val)
    {
      return (((val & 0xff000000) >> 24) |
              ((val & 0x00ff0000) >>  8) |
              ((val & 0x0000ff00) <<  8) |
              ((val & 0x000000ff) << 24));
    }

    // Byte-swap a 64-bit unsigned integer.
    inline std::uint64_t byteswap(std::uint64_t val)
    {
      return (((val & 0xff00000000000000ull) >> 56) |
              ((val & 0x00ff000000000000ull) >> 40) |
              ((val & 0x0000ff0000000000ull) >> 24) |
              ((val & 0x000000ff00000000ull) >>  8) |
              ((val & 0x00000000ff000000ull) <<  8) |
              ((val & 0x0000000000ff0000ull) << 24) |
              ((val & 0x000000000000ff00ull) << 40) |
              ((val & 0x00000000000000ffull) << 56));
    }

    // Rotate 32-bit unsigned integer to the right.
    inline uint32_t rotate_right(uint32_t x, unsigned n)
    {
      return (x >> n) | (x << (32 - n));
    }

    // Rotate 32-bit unsigned integer to the left.
    inline uint32_t rotate_left(uint32_t x, unsigned n)
    {
      return (x << n) | (x >> (32 - n));
    }

    // Rotate 64-bit unsigned integer to the right.
    inline uint64_t rotate_right(uint64_t x, unsigned n)
    {
      return (x >> n) | (x << (64 - n));
    }

    // Rotate 64-bit unsigned integer to the left.
    inline uint64_t rotate_left(uint64_t x, unsigned n)
    {
      return (x << n) | (x >> (64 - n));
    }

    // Clear memory, suppressing compiler optimizations.
    inline void zero_memory(void *v, size_t n)
    {
      volatile unsigned char *p = static_cast<volatile unsigned char *>(v);
      while (n--) *p++ = 0;
    }

    // Clear memory occupied by an array, suppressing compiler optimizations.
    template <typename T, size_t N>
    inline void zero_memory(std::array<T, N>& ar)
    {
      zero_memory(ar.data(), ar.size() * sizeof(T));
    }

    // Clear memory occupied by std::string
    inline void zero_memory(std::string& s)
    {
      if (!s.empty()) zero_memory(&s[0], s.size());
    }

    // Accumulate data and call the transformation function for full blocks.
    template <typename T, typename TF>
    inline void absorb_bytes(const unsigned char* data,
                             size_t len,
                             size_t bs,
                             size_t bschk,
                             unsigned char* m,
                             size_t& pos,
                             T& total,
                             TF transform)
    {
      if (pos && pos + len >= bschk)
      {
        memcpy(m + pos, data, bs - pos);
        transform(m, 1);
        len   -= bs - pos;
        data  += bs - pos;
        total += bs * 8;
        pos = 0;
      }
      if (len >= bschk)
      {
        size_t blocks = (len + bs - bschk) / bs;
        size_t bytes = blocks * bs;
        transform(data, blocks);
        len   -= bytes;
        data  += bytes;
        total += (bytes)* 8;
      }
      memcpy(m + pos, data, len);
      pos += len;
    }

    // Validate that variable hash is within the list of allowed sizes
    inline void validate_hash_size(size_t ahs,
                                   std::initializer_list<size_t> set)
    {
      if (!ahs) throw std::runtime_error("hash size can't be zero");
      if (std::find(set.begin(), set.end(), ahs)) return;
      throw std::runtime_error("invalid hash size");
    }

    // Validate variable hash size up to max bits
    inline void validate_hash_size(size_t ahs,
                                   size_t max)
    {
      if (!ahs) throw std::runtime_error("hash size can't be zero");
      if (ahs % 8) throw std::runtime_error("non-byte hash sizes are not supported");
      if (ahs > max) throw std::runtime_error("invalid hash size");
    }

    template <typename T>
    struct skein_constants
    {
      const static unsigned C4 [8][2];
      const static unsigned C8 [8][4];
      const static unsigned C16[8][8];
      const static unsigned I4 [8][4];
      const static unsigned I8 [8][8];
      const static unsigned I16[8][16];
    };

    template <typename T>
    const unsigned skein_constants<T>::C4[8][2] = {
      { 14, 16 }, { 52, 57 }, { 23, 40 }, {  5, 37 },
      { 25, 33 }, { 46, 12 }, { 58, 22 }, { 32, 32 }
    };

    template <typename T>
    const unsigned skein_constants<T>::C8[8][4] = {
      { 46, 36, 19, 37 }, { 33, 27, 14, 42 },
      { 17, 49, 36, 39 }, { 44,  9, 54, 56 },
      { 39, 30, 34, 24 }, { 13, 50, 10, 17 },
      { 25, 29, 39, 43 }, {  8, 35, 56, 22 }
    };

    template <typename T>
    const unsigned skein_constants<T>::C16[8][8]= {
      { 24, 13,  8, 47,  8, 17, 22, 37 },
      { 38, 19, 10, 55, 49, 18, 23, 52 },
      { 33,  4, 51, 13, 34, 41, 59, 17 },
      {  5, 20, 48, 41, 47, 28, 16, 25 },
      { 41,  9, 37, 31, 12, 47, 44, 30 },
      { 16, 34, 56, 51,  4, 53, 42, 41 },
      { 31, 44, 47, 46, 19, 42, 44, 25 },
      {  9, 48, 35, 52, 23, 31, 37, 20 }
    };

    template <typename T>
    const unsigned skein_constants<T>::I4[8][4]= {
      { 0, 1, 2, 3 }, { 0, 3, 2, 1 },
      { 0, 1, 2, 3 }, { 0, 3, 2, 1 },
      { 0, 1, 2, 3 }, { 0, 3, 2, 1 },
      { 0, 1, 2, 3 }, { 0, 3, 2, 1 }
    };

    template <typename T>
    const unsigned skein_constants<T>::I8[8][8]= {
      { 0, 1, 2, 3, 4, 5, 6, 7 }, { 2, 1, 4, 7, 6, 5, 0, 3 },
      { 4, 1, 6, 3, 0, 5, 2, 7 }, { 6, 1, 0, 7, 2, 5, 4, 3 },
      { 0, 1, 2, 3, 4, 5, 6, 7 }, { 2, 1, 4, 7, 6, 5, 0, 3 },
      { 4, 1, 6, 3, 0, 5, 2, 7 }, { 6, 1, 0, 7, 2, 5, 4, 3 }
    };

    template <typename T>
    const unsigned skein_constants<T>::I16[8][16] = {
      { 0,  1, 2,  3, 4,  5, 6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
      { 0,  9, 2, 13, 6, 11, 4, 15, 10,  7, 12,  3, 14,  5,  8,  1 },
      { 0,  7, 2,  5, 4,  3, 6,  1, 12, 15, 14, 13,  8, 11, 10,  9 },
      { 0, 15, 2, 11, 6, 13, 4,  9, 14,  1,  8,  5, 10,  3, 12,  7 },
      { 0,  1, 2,  3, 4,  5, 6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
      { 0,  9, 2, 13, 6, 11, 4, 15, 10,  7, 12,  3, 14,  5,  8,  1 },
      { 0,  7, 2,  5, 4,  3, 6,  1, 12, 15, 14, 13,  8, 11, 10,  9 },
      { 0, 15, 2, 11, 6, 13, 4,  9, 14,  1,  8,  5, 10,  3, 12,  7 }
    };

    template <size_t N>
    struct skein_functions
    {
    };

    template <>
    struct skein_functions<4>
    {
      template <size_t r>
      static inline void GTv4(uint64_t* G)
      {
        G[skein_constants<void>::I4[r][0]] +=   G[skein_constants<void>::I4[r][1]];
        G[skein_constants<void>::I4[r][1]] = rotate_left(
           G[skein_constants<void>::I4[r][1]],
             skein_constants<void>::C4[r][0]) ^ G[skein_constants<void>::I4[r][0]];
        G[skein_constants<void>::I4[r][2]] +=   G[skein_constants<void>::I4[r][3]];
        G[skein_constants<void>::I4[r][3]] = rotate_left(
           G[skein_constants<void>::I4[r][3]],
             skein_constants<void>::C4[r][1]) ^ G[skein_constants<void>::I4[r][2]];
      }

      template <size_t r>
      static inline void KStRv4(uint64_t* G, uint64_t* keys, uint64_t* tweaks)
      {
        G[0] += keys[(r + 0 + 1) % 5];
        G[1] += keys[(r + 1 + 1) % 5] + tweaks[(r + 1) % 3];
        G[2] += keys[(r + 2 + 1) % 5] + tweaks[(r + 2) % 3];
        G[3] += keys[(r + 3 + 1) % 5] + r + 1;
      }

      template <size_t r>
      static inline void G8(uint64_t* G, uint64_t* keys, uint64_t* tweaks)
      {
        GTv4<0>(G);
        GTv4<1>(G);
        GTv4<2>(G);
        GTv4<3>(G);
        KStRv4<r>(G, keys, tweaks);
        GTv4<4>(G);
        GTv4<5>(G);
        GTv4<6>(G);
        GTv4<7>(G);
        KStRv4<r + 1>(G, keys, tweaks);
      }
    };

    template <>
    struct skein_functions<8>
    {
      template <size_t r>
      static inline void GTv4(uint64_t* G)
      {
        G[skein_constants<void>::I8[r][0]] +=   G[skein_constants<void>::I8[r][1]];
        G[skein_constants<void>::I8[r][1]] = rotate_left(
           G[skein_constants<void>::I8[r][1]],
             skein_constants<void>::C8[r][0]) ^ G[skein_constants<void>::I8[r][0]];
        G[skein_constants<void>::I8[r][2]] +=   G[skein_constants<void>::I8[r][3]];
        G[skein_constants<void>::I8[r][3]] = rotate_left(
           G[skein_constants<void>::I8[r][3]],
             skein_constants<void>::C8[r][1]) ^ G[skein_constants<void>::I8[r][2]];
        G[skein_constants<void>::I8[r][4]] +=   G[skein_constants<void>::I8[r][5]];
        G[skein_constants<void>::I8[r][5]] = rotate_left(
           G[skein_constants<void>::I8[r][5]],
             skein_constants<void>::C8[r][2]) ^ G[skein_constants<void>::I8[r][4]];
        G[skein_constants<void>::I8[r][6]] +=   G[skein_constants<void>::I8[r][7]];
        G[skein_constants<void>::I8[r][7]] = rotate_left(
           G[skein_constants<void>::I8[r][7]],
             skein_constants<void>::C8[r][3]) ^ G[skein_constants<void>::I8[r][6]];
      }

      template <size_t r>
      static inline void KStRv4(uint64_t* G, uint64_t* keys, uint64_t* tweaks)
      {
        G[0] += keys[(r + 0 + 1) % 9];
        G[1] += keys[(r + 1 + 1) % 9];
        G[2] += keys[(r + 2 + 1) % 9];
        G[3] += keys[(r + 3 + 1) % 9];
        G[4] += keys[(r + 4 + 1) % 9];
        G[5] += keys[(r + 5 + 1) % 9] + tweaks[(r + 1) % 3];
        G[6] += keys[(r + 6 + 1) % 9] + tweaks[(r + 2) % 3];
        G[7] += keys[(r + 7 + 1) % 9] + r + 1;
      }

      template <size_t r>
      static inline void G8(uint64_t* G, uint64_t* keys, uint64_t* tweaks)
      {
        GTv4<0>(G);
        GTv4<1>(G);
        GTv4<2>(G);
        GTv4<3>(G);
        KStRv4<r>(G, keys, tweaks);
        GTv4<4>(G);
        GTv4<5>(G);
        GTv4<6>(G);
        GTv4<7>(G);
        KStRv4<r + 1>(G, keys, tweaks);
      }
    };

    template <>
    struct skein_functions<16>
    {

      template <size_t r>
      static inline void GTv4(uint64_t* G)
      {
        G[skein_constants<void>::I16[r][0]] +=   G[skein_constants<void>::I16[r][1]];
        G[skein_constants<void>::I16[r][1]] = rotate_left(
           G[skein_constants<void>::I16[r][1]],
             skein_constants<void>::C16[r][0]) ^ G[skein_constants<void>::I16[r][0]];
        G[skein_constants<void>::I16[r][2]] +=   G[skein_constants<void>::I16[r][3]];
        G[skein_constants<void>::I16[r][3]] = rotate_left(
           G[skein_constants<void>::I16[r][3]],
             skein_constants<void>::C16[r][1]) ^ G[skein_constants<void>::I16[r][2]];
        G[skein_constants<void>::I16[r][4]] +=   G[skein_constants<void>::I16[r][5]];
        G[skein_constants<void>::I16[r][5]] = rotate_left(
           G[skein_constants<void>::I16[r][5]],
             skein_constants<void>::C16[r][2]) ^ G[skein_constants<void>::I16[r][4]];
        G[skein_constants<void>::I16[r][6]] +=   G[skein_constants<void>::I16[r][7]];
        G[skein_constants<void>::I16[r][7]] = rotate_left(
           G[skein_constants<void>::I16[r][7]],
             skein_constants<void>::C16[r][3]) ^ G[skein_constants<void>::I16[r][6]];
        G[skein_constants<void>::I16[r][8]] +=   G[skein_constants<void>::I16[r][9]];
        G[skein_constants<void>::I16[r][9]] = rotate_left(
           G[skein_constants<void>::I16[r][9]],
             skein_constants<void>::C16[r][4]) ^ G[skein_constants<void>::I16[r][8]];
        G[skein_constants<void>::I16[r][10]] +=  G[skein_constants<void>::I16[r][11]];
        G[skein_constants<void>::I16[r][11]] = rotate_left(
           G[skein_constants<void>::I16[r][11]],
             skein_constants<void>::C16[r][5]) ^ G[skein_constants<void>::I16[r][10]];
        G[skein_constants<void>::I16[r][12]] +=  G[skein_constants<void>::I16[r][13]];
        G[skein_constants<void>::I16[r][13]] = rotate_left(
           G[skein_constants<void>::I16[r][13]],
             skein_constants<void>::C16[r][6]) ^ G[skein_constants<void>::I16[r][12]];
        G[skein_constants<void>::I16[r][14]] +=  G[skein_constants<void>::I16[r][15]];
        G[skein_constants<void>::I16[r][15]] = rotate_left(
           G[skein_constants<void>::I16[r][15]],
             skein_constants<void>::C16[r][7]) ^ G[skein_constants<void>::I16[r][14]];
      }

      template <size_t r>
      static inline void KStRv4(uint64_t* G, uint64_t* keys, uint64_t* tweaks)
      {
        G[0]  += keys[(r +  0 + 1) % 17];
        G[1]  += keys[(r +  1 + 1) % 17];
        G[2]  += keys[(r +  2 + 1) % 17];
        G[3]  += keys[(r +  3 + 1) % 17];
        G[4]  += keys[(r +  4 + 1) % 17];
        G[5]  += keys[(r +  5 + 1) % 17];
        G[6]  += keys[(r +  6 + 1) % 17];
        G[7]  += keys[(r +  7 + 1) % 17];
        G[8]  += keys[(r +  8 + 1) % 17];
        G[9]  += keys[(r +  9 + 1) % 17];
        G[10] += keys[(r + 10 + 1) % 17];
        G[11] += keys[(r + 11 + 1) % 17];
        G[12] += keys[(r + 12 + 1) % 17];
        G[13] += keys[(r + 13 + 1) % 17] + tweaks[(r + 1) % 3];
        G[14] += keys[(r + 14 + 1) % 17] + tweaks[(r + 2) % 3];
        G[15] += keys[(r + 15 + 1) % 17] + r + 1;
      }

      template <size_t r>
      static inline void G8(uint64_t* G, uint64_t* keys, uint64_t* tweaks)
      {
        GTv4<0>(G);
        GTv4<1>(G);
        GTv4<2>(G);
        GTv4<3>(G);
        KStRv4<r>(G, keys, tweaks);
        GTv4<4>(G);
        GTv4<5>(G);
        GTv4<6>(G);
        GTv4<7>(G);
        KStRv4<r + 1>(G, keys, tweaks);
      }
    };

    template <size_t N>
    class skein_provider
    {
     public:
      static const bool is_xof = false;

      skein_provider(size_t hashsize = N)
      : hs{hashsize}
      {
        validate_hash_size(hashsize, SIZE_MAX);
      }

      ~skein_provider()
      {
        clear();
      }

      inline void set_personalization(const std::string& personalization)
      {
        p = personalization;
      }

      inline void set_nonce(const std::string& nonce)
      {
        n = nonce;
      }

      inline void set_key(const std::string& key)
      {
        k = key;
      }

      inline void init()
      {
        squeezing = false;
        tweak[0] = 0ULL;
        pos = 0;
        total = 0;
        zero_memory(H);
        inject_parameter(k, 0ULL);
        tweak[1] = (1ULL << 62) | (4ULL << 56) | (1ULL << 63);
        zero_memory(m);
        m[0] = 0x53;
        m[1] = 0x48;
        m[2] = 0x41;
        m[3] = 0x33;
        m[4] = 0x01;
        uint64_t size64 = is_xof ? static_cast<uint64_t>(-1) : hs;
        memcpy(&m[8], &size64, 8);
        transform(m.data(), 1, 32);
        pos = 0;
        total = 0;
        tweak[0] = 0ULL;
        tweak[1] = (1ULL << 62) | (48ULL << 56);
        inject_parameter(p, 8ULL);
        inject_parameter(n, 20ULL);
      }

      inline void update(const unsigned char* data, size_t len)
      {
        absorb_bytes(data, len,
          N / 8, N / 8 + 1,
          m.data(), pos, total,
          [this](const unsigned char* adata, size_t alen)
                { transform(adata, alen, N / 8); });
      }

      inline void squeeze(unsigned char* hash, size_t ahs)
      {
        size_t processed = 0;
        if (!squeezing)
        {
          squeezing = true;
          tweak[1] |= 1ull << 63; // last block
          if (pos < N / 8)
                  memset(&m[pos], 0, N / 8 - pos);

          transform(m.data(), 1, pos);
          memset(&m[0], 0, N / 8);
          memcpy(&hbk[0], H.data(), N / 8);
          pos = 0;
          total = 0;
        }
        else if (pos < N / 8)
        {
          size_t to_copy = std::min(ahs, N / 8 - pos);
          memcpy(hash, reinterpret_cast<unsigned char*>(H.data()) + pos, to_copy);
          processed += to_copy;
          pos += to_copy;
        }
        while (processed < ahs)
        {
          pos = std::min(ahs - processed, N / 8);
          tweak[0] = 0;
          tweak[1] = 255ULL << 56;
          memcpy(&m[0], &total, 8);
          memcpy(&H[0], hbk.data(), N / 8);
          transform(m.data(), 1, 8);
          memcpy(hash + processed, H.data(), pos);
          processed += pos;
          ++total;
        }
      }

      inline void _final(unsigned char* hash)
      {
        return squeeze(hash, hs / 8);
      }

      inline size_t hash_size() const
      {
        return hs;
      }

      inline void clear()
      {
        zero_memory(H);
        zero_memory(hbk);
        zero_memory(m);
        zero_memory(tweak);
        zero_memory(p);
        zero_memory(n);
        zero_memory(k);
        p.clear();
        n.clear();
        k.clear();
      }

     private:

      inline void transform(const unsigned char* mp, uint64_t num_blks, size_t reallen)
      {
        uint64_t keys[N / 64 + 1];
        uint64_t tweaks[3];

        for (uint64_t b = 0; b < num_blks; b++)
        {
          uint64_t M[N / 64];
          uint64_t G[N / 64];
          for (uint64_t i = 0; i < N / 64; i++)
            M[i] = (reinterpret_cast<const uint64_t*>(mp)[b * N / 64 + i]);
          memcpy(keys, H.data(), sizeof(uint64_t) * N / 64);
          memcpy(tweaks, tweak.data(), sizeof(uint64_t) * 2);
          tweaks[0] += reallen;
          tweaks[2] = tweaks[0] ^ tweaks[1];
          keys[N / 64] = 0x1BD11BDAA9FC1A22ULL;
          for (int i = 0; i < N / 64; i++)
          {
            keys[N / 64] ^= keys[i];
            G[i] = M[i] + keys[i];
          }
          G[N / 64 - 3] += tweaks[0];
          G[N / 64 - 2] += tweaks[1];

          skein_functions<N / 64>::template G8<0>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<2>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<4>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<6>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<8>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<10>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<12>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<14>(G, keys, tweaks);
          skein_functions<N / 64>::template G8<16>(G, keys, tweaks);
          if (N == 1024)
            skein_functions<N / 64>::template G8<18>(G, keys, tweaks);

          tweaks[1] &= ~(64ULL << 56);
          tweak[0] = tweaks[0];
          tweak[1] = tweaks[1];

          for (int i = 0; i < N / 64; i++) H[i] = G[i] ^ M[i];
        }
      }

      inline void inject_parameter(const std::string& kpn, uint64_t code)
      {
        if (kpn.empty()) return;

        tweak[1] = (1ULL << 62) | (code << 56);
        update(reinterpret_cast<const unsigned char*>(kpn.data()), kpn.length());
        tweak[1] |= 1ull << 63; // last block
        if (pos < N / 8) memset(&m[pos], 0, N / 8 - pos);
        transform(m.data(), 1, pos);
        squeezing = false;
        pos = 0;
        total = 0;
        tweak[0] = 0ULL;
        tweak[1] = (1ULL << 62) | (48ULL << 56);
      }

      std::array<uint64_t, N / 64> H, hbk;
      std::array<unsigned char, N / 8> m;
      size_t pos;
      uint64_t total;
      std::array<uint64_t, 2> tweak;
      size_t hs;
      bool squeezing;
      std::string p, n, k;

    }; // class skein_provider

    template <typename V>
    struct groestl_constants
    {
      static const uint64_t T[8][256];
    };

    template <typename V>
    const uint64_t groestl_constants<V>::T[8][256] =
     {
      {
        0xc6a597f4a5f432c6ull, 0xf884eb9784976ff8ull, 0xee99c7b099b05eeeull, 0xf68df78c8d8c7af6ull,
        0xff0de5170d17e8ffull, 0xd6bdb7dcbddc0ad6ull, 0xdeb1a7c8b1c816deull, 0x915439fc54fc6d91ull,
        0x6050c0f050f09060ull, 0x0203040503050702ull, 0xcea987e0a9e02eceull, 0x567dac877d87d156ull,
        0xe719d52b192bcce7ull, 0xb56271a662a613b5ull, 0x4de69a31e6317c4dull, 0xec9ac3b59ab559ecull,
        0x8f4505cf45cf408full, 0x1f9d3ebc9dbca31full, 0x894009c040c04989ull, 0xfa87ef92879268faull,
        0xef15c53f153fd0efull, 0xb2eb7f26eb2694b2ull, 0x8ec90740c940ce8eull, 0xfb0bed1d0b1de6fbull,
        0x41ec822fec2f6e41ull, 0xb3677da967a91ab3ull, 0x5ffdbe1cfd1c435full, 0x45ea8a25ea256045ull,
        0x23bf46dabfdaf923ull, 0x53f7a602f7025153ull, 0xe496d3a196a145e4ull, 0x9b5b2ded5bed769bull,
        0x75c2ea5dc25d2875ull, 0xe11cd9241c24c5e1ull, 0x3dae7ae9aee9d43dull, 0x4c6a98be6abef24cull,
        0x6c5ad8ee5aee826cull, 0x7e41fcc341c3bd7eull, 0xf502f1060206f3f5ull, 0x834f1dd14fd15283ull,
        0x685cd0e45ce48c68ull, 0x51f4a207f4075651ull, 0xd134b95c345c8dd1ull, 0xf908e9180818e1f9ull,
        0xe293dfae93ae4ce2ull, 0xab734d9573953eabull, 0x6253c4f553f59762ull, 0x2a3f54413f416b2aull,
        0x080c10140c141c08ull, 0x955231f652f66395ull, 0x46658caf65afe946ull, 0x9d5e21e25ee27f9dull,
        0x3028607828784830ull, 0x37a16ef8a1f8cf37ull, 0x0a0f14110f111b0aull, 0x2fb55ec4b5c4eb2full,
        0x0e091c1b091b150eull, 0x2436485a365a7e24ull, 0x1b9b36b69bb6ad1bull, 0xdf3da5473d4798dfull,
        0xcd26816a266aa7cdull, 0x4e699cbb69bbf54eull, 0x7fcdfe4ccd4c337full, 0xea9fcfba9fba50eaull,
        0x121b242d1b2d3f12ull, 0x1d9e3ab99eb9a41dull, 0x5874b09c749cc458ull, 0x342e68722e724634ull,
        0x362d6c772d774136ull, 0xdcb2a3cdb2cd11dcull, 0xb4ee7329ee299db4ull, 0x5bfbb616fb164d5bull,
        0xa4f65301f601a5a4ull, 0x764decd74dd7a176ull, 0xb76175a361a314b7ull, 0x7dcefa49ce49347dull,
        0x527ba48d7b8ddf52ull, 0xdd3ea1423e429fddull, 0x5e71bc937193cd5eull, 0x139726a297a2b113ull,
        0xa6f55704f504a2a6ull, 0xb96869b868b801b9ull, 0x0000000000000000ull, 0xc12c99742c74b5c1ull,
        0x406080a060a0e040ull, 0xe31fdd211f21c2e3ull, 0x79c8f243c8433a79ull, 0xb6ed772ced2c9ab6ull,
        0xd4beb3d9bed90dd4ull, 0x8d4601ca46ca478dull, 0x67d9ce70d9701767ull, 0x724be4dd4bddaf72ull,
        0x94de3379de79ed94ull, 0x98d42b67d467ff98ull, 0xb0e87b23e82393b0ull, 0x854a11de4ade5b85ull,
        0xbb6b6dbd6bbd06bbull, 0xc52a917e2a7ebbc5ull, 0x4fe59e34e5347b4full, 0xed16c13a163ad7edull,
        0x86c51754c554d286ull, 0x9ad72f62d762f89aull, 0x6655ccff55ff9966ull, 0x119422a794a7b611ull,
        0x8acf0f4acf4ac08aull, 0xe910c9301030d9e9ull, 0x0406080a060a0e04ull, 0xfe81e798819866feull,
        0xa0f05b0bf00baba0ull, 0x7844f0cc44ccb478ull, 0x25ba4ad5bad5f025ull, 0x4be3963ee33e754bull,
        0xa2f35f0ef30eaca2ull, 0x5dfeba19fe19445dull, 0x80c01b5bc05bdb80ull, 0x058a0a858a858005ull,
        0x3fad7eecadecd33full, 0x21bc42dfbcdffe21ull, 0x7048e0d848d8a870ull, 0xf104f90c040cfdf1ull,
        0x63dfc67adf7a1963ull, 0x77c1ee58c1582f77ull, 0xaf75459f759f30afull, 0x426384a563a5e742ull,
        0x2030405030507020ull, 0xe51ad12e1a2ecbe5ull, 0xfd0ee1120e12effdull, 0xbf6d65b76db708bfull,
        0x814c19d44cd45581ull, 0x1814303c143c2418ull, 0x26354c5f355f7926ull, 0xc32f9d712f71b2c3ull,
        0xbee16738e13886beull, 0x35a26afda2fdc835ull, 0x88cc0b4fcc4fc788ull, 0x2e395c4b394b652eull,
        0x93573df957f96a93ull, 0x55f2aa0df20d5855ull, 0xfc82e39d829d61fcull, 0x7a47f4c947c9b37aull,
        0xc8ac8befacef27c8ull, 0xbae76f32e73288baull, 0x322b647d2b7d4f32ull, 0xe695d7a495a442e6ull,
        0xc0a09bfba0fb3bc0ull, 0x199832b398b3aa19ull, 0x9ed12768d168f69eull, 0xa37f5d817f8122a3ull,
        0x446688aa66aaee44ull, 0x547ea8827e82d654ull, 0x3bab76e6abe6dd3bull, 0x0b83169e839e950bull,
        0x8cca0345ca45c98cull, 0xc729957b297bbcc7ull, 0x6bd3d66ed36e056bull, 0x283c50443c446c28ull,
        0xa779558b798b2ca7ull, 0xbce2633de23d81bcull, 0x161d2c271d273116ull, 0xad76419a769a37adull,
        0xdb3bad4d3b4d96dbull, 0x6456c8fa56fa9e64ull, 0x744ee8d24ed2a674ull, 0x141e28221e223614ull,
        0x92db3f76db76e492ull, 0x0c0a181e0a1e120cull, 0x486c90b46cb4fc48ull, 0xb8e46b37e4378fb8ull,
        0x9f5d25e75de7789full, 0xbd6e61b26eb20fbdull, 0x43ef862aef2a6943ull, 0xc4a693f1a6f135c4ull,
        0x39a872e3a8e3da39ull, 0x31a462f7a4f7c631ull, 0xd337bd5937598ad3ull, 0xf28bff868b8674f2ull,
        0xd532b156325683d5ull, 0x8b430dc543c54e8bull, 0x6e59dceb59eb856eull, 0xdab7afc2b7c218daull,
        0x018c028f8c8f8e01ull, 0xb16479ac64ac1db1ull, 0x9cd2236dd26df19cull, 0x49e0923be03b7249ull,
        0xd8b4abc7b4c71fd8ull, 0xacfa4315fa15b9acull, 0xf307fd090709faf3ull, 0xcf25856f256fa0cfull,
        0xcaaf8feaafea20caull, 0xf48ef3898e897df4ull, 0x47e98e20e9206747ull, 0x1018202818283810ull,
        0x6fd5de64d5640b6full, 0xf088fb83888373f0ull, 0x4a6f94b16fb1fb4aull, 0x5c72b8967296ca5cull,
        0x3824706c246c5438ull, 0x57f1ae08f1085f57ull, 0x73c7e652c7522173ull, 0x975135f351f36497ull,
        0xcb238d652365aecbull, 0xa17c59847c8425a1ull, 0xe89ccbbf9cbf57e8ull, 0x3e217c6321635d3eull,
        0x96dd377cdd7cea96ull, 0x61dcc27fdc7f1e61ull, 0x0d861a9186919c0dull, 0x0f851e9485949b0full,
        0xe090dbab90ab4be0ull, 0x7c42f8c642c6ba7cull, 0x71c4e257c4572671ull, 0xccaa83e5aae529ccull,
        0x90d83b73d873e390ull, 0x06050c0f050f0906ull, 0xf701f5030103f4f7ull, 0x1c12383612362a1cull,
        0xc2a39ffea3fe3cc2ull, 0x6a5fd4e15fe18b6aull, 0xaef94710f910beaeull, 0x69d0d26bd06b0269ull,
        0x17912ea891a8bf17ull, 0x995829e858e87199ull, 0x3a2774692769533aull, 0x27b94ed0b9d0f727ull,
        0xd938a948384891d9ull, 0xeb13cd351335deebull, 0x2bb356ceb3cee52bull, 0x2233445533557722ull,
        0xd2bbbfd6bbd604d2ull, 0xa9704990709039a9ull, 0x07890e8089808707ull, 0x33a766f2a7f2c133ull,
        0x2db65ac1b6c1ec2dull, 0x3c22786622665a3cull, 0x15922aad92adb815ull, 0xc92089602060a9c9ull,
        0x874915db49db5c87ull, 0xaaff4f1aff1ab0aaull, 0x5078a0887888d850ull, 0xa57a518e7a8e2ba5ull,
        0x038f068a8f8a8903ull, 0x59f8b213f8134a59ull, 0x0980129b809b9209ull, 0x1a1734391739231aull,
        0x65daca75da751065ull, 0xd731b553315384d7ull, 0x84c61351c651d584ull, 0xd0b8bbd3b8d303d0ull,
        0x82c31f5ec35edc82ull, 0x29b052cbb0cbe229ull, 0x5a77b4997799c35aull, 0x1e113c3311332d1eull,
        0x7bcbf646cb463d7bull, 0xa8fc4b1ffc1fb7a8ull, 0x6dd6da61d6610c6dull, 0x2c3a584e3a4e622cull
      },
      {
        0xa597f4a5f432c6c6ull, 0x84eb9784976ff8f8ull, 0x99c7b099b05eeeeeull, 0x8df78c8d8c7af6f6ull,
        0x0de5170d17e8ffffull, 0xbdb7dcbddc0ad6d6ull, 0xb1a7c8b1c816dedeull, 0x5439fc54fc6d9191ull,
        0x50c0f050f0906060ull, 0x0304050305070202ull, 0xa987e0a9e02ececeull, 0x7dac877d87d15656ull,
        0x19d52b192bcce7e7ull, 0x6271a662a613b5b5ull, 0xe69a31e6317c4d4dull, 0x9ac3b59ab559ececull,
        0x4505cf45cf408f8full, 0x9d3ebc9dbca31f1full, 0x4009c040c0498989ull, 0x87ef92879268fafaull,
        0x15c53f153fd0efefull, 0xeb7f26eb2694b2b2ull, 0xc90740c940ce8e8eull, 0x0bed1d0b1de6fbfbull,
        0xec822fec2f6e4141ull, 0x677da967a91ab3b3ull, 0xfdbe1cfd1c435f5full, 0xea8a25ea25604545ull,
        0xbf46dabfdaf92323ull, 0xf7a602f702515353ull, 0x96d3a196a145e4e4ull, 0x5b2ded5bed769b9bull,
        0xc2ea5dc25d287575ull, 0x1cd9241c24c5e1e1ull, 0xae7ae9aee9d43d3dull, 0x6a98be6abef24c4cull,
        0x5ad8ee5aee826c6cull, 0x41fcc341c3bd7e7eull, 0x02f1060206f3f5f5ull, 0x4f1dd14fd1528383ull,
        0x5cd0e45ce48c6868ull, 0xf4a207f407565151ull, 0x34b95c345c8dd1d1ull, 0x08e9180818e1f9f9ull,
        0x93dfae93ae4ce2e2ull, 0x734d9573953eababull, 0x53c4f553f5976262ull, 0x3f54413f416b2a2aull,
        0x0c10140c141c0808ull, 0x5231f652f6639595ull, 0x658caf65afe94646ull, 0x5e21e25ee27f9d9dull,
        0x2860782878483030ull, 0xa16ef8a1f8cf3737ull, 0x0f14110f111b0a0aull, 0xb55ec4b5c4eb2f2full,
        0x091c1b091b150e0eull, 0x36485a365a7e2424ull, 0x9b36b69bb6ad1b1bull, 0x3da5473d4798dfdfull,
        0x26816a266aa7cdcdull, 0x699cbb69bbf54e4eull, 0xcdfe4ccd4c337f7full, 0x9fcfba9fba50eaeaull,
        0x1b242d1b2d3f1212ull, 0x9e3ab99eb9a41d1dull, 0x74b09c749cc45858ull, 0x2e68722e72463434ull,
        0x2d6c772d77413636ull, 0xb2a3cdb2cd11dcdcull, 0xee7329ee299db4b4ull, 0xfbb616fb164d5b5bull,
        0xf65301f601a5a4a4ull, 0x4decd74dd7a17676ull, 0x6175a361a314b7b7ull, 0xcefa49ce49347d7dull,
        0x7ba48d7b8ddf5252ull, 0x3ea1423e429fddddull, 0x71bc937193cd5e5eull, 0x9726a297a2b11313ull,
        0xf55704f504a2a6a6ull, 0x6869b868b801b9b9ull, 0x0000000000000000ull, 0x2c99742c74b5c1c1ull,
        0x6080a060a0e04040ull, 0x1fdd211f21c2e3e3ull, 0xc8f243c8433a7979ull, 0xed772ced2c9ab6b6ull,
        0xbeb3d9bed90dd4d4ull, 0x4601ca46ca478d8dull, 0xd9ce70d970176767ull, 0x4be4dd4bddaf7272ull,
        0xde3379de79ed9494ull, 0xd42b67d467ff9898ull, 0xe87b23e82393b0b0ull, 0x4a11de4ade5b8585ull,
        0x6b6dbd6bbd06bbbbull, 0x2a917e2a7ebbc5c5ull, 0xe59e34e5347b4f4full, 0x16c13a163ad7ededull,
        0xc51754c554d28686ull, 0xd72f62d762f89a9aull, 0x55ccff55ff996666ull, 0x9422a794a7b61111ull,
        0xcf0f4acf4ac08a8aull, 0x10c9301030d9e9e9ull, 0x06080a060a0e0404ull, 0x81e798819866fefeull,
        0xf05b0bf00baba0a0ull, 0x44f0cc44ccb47878ull, 0xba4ad5bad5f02525ull, 0xe3963ee33e754b4bull,
        0xf35f0ef30eaca2a2ull, 0xfeba19fe19445d5dull, 0xc01b5bc05bdb8080ull, 0x8a0a858a85800505ull,
        0xad7eecadecd33f3full, 0xbc42dfbcdffe2121ull, 0x48e0d848d8a87070ull, 0x04f90c040cfdf1f1ull,
        0xdfc67adf7a196363ull, 0xc1ee58c1582f7777ull, 0x75459f759f30afafull, 0x6384a563a5e74242ull,
        0x3040503050702020ull, 0x1ad12e1a2ecbe5e5ull, 0x0ee1120e12effdfdull, 0x6d65b76db708bfbfull,
        0x4c19d44cd4558181ull, 0x14303c143c241818ull, 0x354c5f355f792626ull, 0x2f9d712f71b2c3c3ull,
        0xe16738e13886bebeull, 0xa26afda2fdc83535ull, 0xcc0b4fcc4fc78888ull, 0x395c4b394b652e2eull,
        0x573df957f96a9393ull, 0xf2aa0df20d585555ull, 0x82e39d829d61fcfcull, 0x47f4c947c9b37a7aull,
        0xac8befacef27c8c8ull, 0xe76f32e73288babaull, 0x2b647d2b7d4f3232ull, 0x95d7a495a442e6e6ull,
        0xa09bfba0fb3bc0c0ull, 0x9832b398b3aa1919ull, 0xd12768d168f69e9eull, 0x7f5d817f8122a3a3ull,
        0x6688aa66aaee4444ull, 0x7ea8827e82d65454ull, 0xab76e6abe6dd3b3bull, 0x83169e839e950b0bull,
        0xca0345ca45c98c8cull, 0x29957b297bbcc7c7ull, 0xd3d66ed36e056b6bull, 0x3c50443c446c2828ull,
        0x79558b798b2ca7a7ull, 0xe2633de23d81bcbcull, 0x1d2c271d27311616ull, 0x76419a769a37adadull,
        0x3bad4d3b4d96dbdbull, 0x56c8fa56fa9e6464ull, 0x4ee8d24ed2a67474ull, 0x1e28221e22361414ull,
        0xdb3f76db76e49292ull, 0x0a181e0a1e120c0cull, 0x6c90b46cb4fc4848ull, 0xe46b37e4378fb8b8ull,
        0x5d25e75de7789f9full, 0x6e61b26eb20fbdbdull, 0xef862aef2a694343ull, 0xa693f1a6f135c4c4ull,
        0xa872e3a8e3da3939ull, 0xa462f7a4f7c63131ull, 0x37bd5937598ad3d3ull, 0x8bff868b8674f2f2ull,
        0x32b156325683d5d5ull, 0x430dc543c54e8b8bull, 0x59dceb59eb856e6eull, 0xb7afc2b7c218dadaull,
        0x8c028f8c8f8e0101ull, 0x6479ac64ac1db1b1ull, 0xd2236dd26df19c9cull, 0xe0923be03b724949ull,
        0xb4abc7b4c71fd8d8ull, 0xfa4315fa15b9acacull, 0x07fd090709faf3f3ull, 0x25856f256fa0cfcfull,
        0xaf8feaafea20cacaull, 0x8ef3898e897df4f4ull, 0xe98e20e920674747ull, 0x1820281828381010ull,
        0xd5de64d5640b6f6full, 0x88fb83888373f0f0ull, 0x6f94b16fb1fb4a4aull, 0x72b8967296ca5c5cull,
        0x24706c246c543838ull, 0xf1ae08f1085f5757ull, 0xc7e652c752217373ull, 0x5135f351f3649797ull,
        0x238d652365aecbcbull, 0x7c59847c8425a1a1ull, 0x9ccbbf9cbf57e8e8ull, 0x217c6321635d3e3eull,
        0xdd377cdd7cea9696ull, 0xdcc27fdc7f1e6161ull, 0x861a9186919c0d0dull, 0x851e9485949b0f0full,
        0x90dbab90ab4be0e0ull, 0x42f8c642c6ba7c7cull, 0xc4e257c457267171ull, 0xaa83e5aae529ccccull,
        0xd83b73d873e39090ull, 0x050c0f050f090606ull, 0x01f5030103f4f7f7ull, 0x12383612362a1c1cull,
        0xa39ffea3fe3cc2c2ull, 0x5fd4e15fe18b6a6aull, 0xf94710f910beaeaeull, 0xd0d26bd06b026969ull,
        0x912ea891a8bf1717ull, 0x5829e858e8719999ull, 0x2774692769533a3aull, 0xb94ed0b9d0f72727ull,
        0x38a948384891d9d9ull, 0x13cd351335deebebull, 0xb356ceb3cee52b2bull, 0x3344553355772222ull,
        0xbbbfd6bbd604d2d2ull, 0x704990709039a9a9ull, 0x890e808980870707ull, 0xa766f2a7f2c13333ull,
        0xb65ac1b6c1ec2d2dull, 0x22786622665a3c3cull, 0x922aad92adb81515ull, 0x2089602060a9c9c9ull,
        0x4915db49db5c8787ull, 0xff4f1aff1ab0aaaaull, 0x78a0887888d85050ull, 0x7a518e7a8e2ba5a5ull,
        0x8f068a8f8a890303ull, 0xf8b213f8134a5959ull, 0x80129b809b920909ull, 0x1734391739231a1aull,
        0xdaca75da75106565ull, 0x31b553315384d7d7ull, 0xc61351c651d58484ull, 0xb8bbd3b8d303d0d0ull,
        0xc31f5ec35edc8282ull, 0xb052cbb0cbe22929ull, 0x77b4997799c35a5aull, 0x113c3311332d1e1eull,
        0xcbf646cb463d7b7bull, 0xfc4b1ffc1fb7a8a8ull, 0xd6da61d6610c6d6dull, 0x3a584e3a4e622c2cull
      },
      {
        0x97f4a5f432c6c6a5ull, 0xeb9784976ff8f884ull, 0xc7b099b05eeeee99ull, 0xf78c8d8c7af6f68dull,
        0xe5170d17e8ffff0dull, 0xb7dcbddc0ad6d6bdull, 0xa7c8b1c816dedeb1ull, 0x39fc54fc6d919154ull,
        0xc0f050f090606050ull, 0x0405030507020203ull, 0x87e0a9e02ececea9ull, 0xac877d87d156567dull,
        0xd52b192bcce7e719ull, 0x71a662a613b5b562ull, 0x9a31e6317c4d4de6ull, 0xc3b59ab559ecec9aull,
        0x05cf45cf408f8f45ull, 0x3ebc9dbca31f1f9dull, 0x09c040c049898940ull, 0xef92879268fafa87ull,
        0xc53f153fd0efef15ull, 0x7f26eb2694b2b2ebull, 0x0740c940ce8e8ec9ull, 0xed1d0b1de6fbfb0bull,
        0x822fec2f6e4141ecull, 0x7da967a91ab3b367ull, 0xbe1cfd1c435f5ffdull, 0x8a25ea25604545eaull,
        0x46dabfdaf92323bfull, 0xa602f702515353f7ull, 0xd3a196a145e4e496ull, 0x2ded5bed769b9b5bull,
        0xea5dc25d287575c2ull, 0xd9241c24c5e1e11cull, 0x7ae9aee9d43d3daeull, 0x98be6abef24c4c6aull,
        0xd8ee5aee826c6c5aull, 0xfcc341c3bd7e7e41ull, 0xf1060206f3f5f502ull, 0x1dd14fd15283834full,
        0xd0e45ce48c68685cull, 0xa207f407565151f4ull, 0xb95c345c8dd1d134ull, 0xe9180818e1f9f908ull,
        0xdfae93ae4ce2e293ull, 0x4d9573953eabab73ull, 0xc4f553f597626253ull, 0x54413f416b2a2a3full,
        0x10140c141c08080cull, 0x31f652f663959552ull, 0x8caf65afe9464665ull, 0x21e25ee27f9d9d5eull,
        0x6078287848303028ull, 0x6ef8a1f8cf3737a1ull, 0x14110f111b0a0a0full, 0x5ec4b5c4eb2f2fb5ull,
        0x1c1b091b150e0e09ull, 0x485a365a7e242436ull, 0x36b69bb6ad1b1b9bull, 0xa5473d4798dfdf3dull,
        0x816a266aa7cdcd26ull, 0x9cbb69bbf54e4e69ull, 0xfe4ccd4c337f7fcdull, 0xcfba9fba50eaea9full,
        0x242d1b2d3f12121bull, 0x3ab99eb9a41d1d9eull, 0xb09c749cc4585874ull, 0x68722e724634342eull,
        0x6c772d774136362dull, 0xa3cdb2cd11dcdcb2ull, 0x7329ee299db4b4eeull, 0xb616fb164d5b5bfbull,
        0x5301f601a5a4a4f6ull, 0xecd74dd7a176764dull, 0x75a361a314b7b761ull, 0xfa49ce49347d7dceull,
        0xa48d7b8ddf52527bull, 0xa1423e429fdddd3eull, 0xbc937193cd5e5e71ull, 0x26a297a2b1131397ull,
        0x5704f504a2a6a6f5ull, 0x69b868b801b9b968ull, 0x0000000000000000ull, 0x99742c74b5c1c12cull,
        0x80a060a0e0404060ull, 0xdd211f21c2e3e31full, 0xf243c8433a7979c8ull, 0x772ced2c9ab6b6edull,
        0xb3d9bed90dd4d4beull, 0x01ca46ca478d8d46ull, 0xce70d970176767d9ull, 0xe4dd4bddaf72724bull,
        0x3379de79ed9494deull, 0x2b67d467ff9898d4ull, 0x7b23e82393b0b0e8ull, 0x11de4ade5b85854aull,
        0x6dbd6bbd06bbbb6bull, 0x917e2a7ebbc5c52aull, 0x9e34e5347b4f4fe5ull, 0xc13a163ad7eded16ull,
        0x1754c554d28686c5ull, 0x2f62d762f89a9ad7ull, 0xccff55ff99666655ull, 0x22a794a7b6111194ull,
        0x0f4acf4ac08a8acfull, 0xc9301030d9e9e910ull, 0x080a060a0e040406ull, 0xe798819866fefe81ull,
        0x5b0bf00baba0a0f0ull, 0xf0cc44ccb4787844ull, 0x4ad5bad5f02525baull, 0x963ee33e754b4be3ull,
        0x5f0ef30eaca2a2f3ull, 0xba19fe19445d5dfeull, 0x1b5bc05bdb8080c0ull, 0x0a858a858005058aull,
        0x7eecadecd33f3fadull, 0x42dfbcdffe2121bcull, 0xe0d848d8a8707048ull, 0xf90c040cfdf1f104ull,
        0xc67adf7a196363dfull, 0xee58c1582f7777c1ull, 0x459f759f30afaf75ull, 0x84a563a5e7424263ull,
        0x4050305070202030ull, 0xd12e1a2ecbe5e51aull, 0xe1120e12effdfd0eull, 0x65b76db708bfbf6dull,
        0x19d44cd45581814cull, 0x303c143c24181814ull, 0x4c5f355f79262635ull, 0x9d712f71b2c3c32full,
        0x6738e13886bebee1ull, 0x6afda2fdc83535a2ull, 0x0b4fcc4fc78888ccull, 0x5c4b394b652e2e39ull,
        0x3df957f96a939357ull, 0xaa0df20d585555f2ull, 0xe39d829d61fcfc82ull, 0xf4c947c9b37a7a47ull,
        0x8befacef27c8c8acull, 0x6f32e73288babae7ull, 0x647d2b7d4f32322bull, 0xd7a495a442e6e695ull,
        0x9bfba0fb3bc0c0a0ull, 0x32b398b3aa191998ull, 0x2768d168f69e9ed1ull, 0x5d817f8122a3a37full,
        0x88aa66aaee444466ull, 0xa8827e82d654547eull, 0x76e6abe6dd3b3babull, 0x169e839e950b0b83ull,
        0x0345ca45c98c8ccaull, 0x957b297bbcc7c729ull, 0xd66ed36e056b6bd3ull, 0x50443c446c28283cull,
        0x558b798b2ca7a779ull, 0x633de23d81bcbce2ull, 0x2c271d273116161dull, 0x419a769a37adad76ull,
        0xad4d3b4d96dbdb3bull, 0xc8fa56fa9e646456ull, 0xe8d24ed2a674744eull, 0x28221e223614141eull,
        0x3f76db76e49292dbull, 0x181e0a1e120c0c0aull, 0x90b46cb4fc48486cull, 0x6b37e4378fb8b8e4ull,
        0x25e75de7789f9f5dull, 0x61b26eb20fbdbd6eull, 0x862aef2a694343efull, 0x93f1a6f135c4c4a6ull,
        0x72e3a8e3da3939a8ull, 0x62f7a4f7c63131a4ull, 0xbd5937598ad3d337ull, 0xff868b8674f2f28bull,
        0xb156325683d5d532ull, 0x0dc543c54e8b8b43ull, 0xdceb59eb856e6e59ull, 0xafc2b7c218dadab7ull,
        0x028f8c8f8e01018cull, 0x79ac64ac1db1b164ull, 0x236dd26df19c9cd2ull, 0x923be03b724949e0ull,
        0xabc7b4c71fd8d8b4ull, 0x4315fa15b9acacfaull, 0xfd090709faf3f307ull, 0x856f256fa0cfcf25ull,
        0x8feaafea20cacaafull, 0xf3898e897df4f48eull, 0x8e20e920674747e9ull, 0x2028182838101018ull,
        0xde64d5640b6f6fd5ull, 0xfb83888373f0f088ull, 0x94b16fb1fb4a4a6full, 0xb8967296ca5c5c72ull,
        0x706c246c54383824ull, 0xae08f1085f5757f1ull, 0xe652c752217373c7ull, 0x35f351f364979751ull,
        0x8d652365aecbcb23ull, 0x59847c8425a1a17cull, 0xcbbf9cbf57e8e89cull, 0x7c6321635d3e3e21ull,
        0x377cdd7cea9696ddull, 0xc27fdc7f1e6161dcull, 0x1a9186919c0d0d86ull, 0x1e9485949b0f0f85ull,
        0xdbab90ab4be0e090ull, 0xf8c642c6ba7c7c42ull, 0xe257c457267171c4ull, 0x83e5aae529ccccaaull,
        0x3b73d873e39090d8ull, 0x0c0f050f09060605ull, 0xf5030103f4f7f701ull, 0x383612362a1c1c12ull,
        0x9ffea3fe3cc2c2a3ull, 0xd4e15fe18b6a6a5full, 0x4710f910beaeaef9ull, 0xd26bd06b026969d0ull,
        0x2ea891a8bf171791ull, 0x29e858e871999958ull, 0x74692769533a3a27ull, 0x4ed0b9d0f72727b9ull,
        0xa948384891d9d938ull, 0xcd351335deebeb13ull, 0x56ceb3cee52b2bb3ull, 0x4455335577222233ull,
        0xbfd6bbd604d2d2bbull, 0x4990709039a9a970ull, 0x0e80898087070789ull, 0x66f2a7f2c13333a7ull,
        0x5ac1b6c1ec2d2db6ull, 0x786622665a3c3c22ull, 0x2aad92adb8151592ull, 0x89602060a9c9c920ull,
        0x15db49db5c878749ull, 0x4f1aff1ab0aaaaffull, 0xa0887888d8505078ull, 0x518e7a8e2ba5a57aull,
        0x068a8f8a8903038full, 0xb213f8134a5959f8ull, 0x129b809b92090980ull, 0x34391739231a1a17ull,
        0xca75da75106565daull, 0xb553315384d7d731ull, 0x1351c651d58484c6ull, 0xbbd3b8d303d0d0b8ull,
        0x1f5ec35edc8282c3ull, 0x52cbb0cbe22929b0ull, 0xb4997799c35a5a77ull, 0x3c3311332d1e1e11ull,
        0xf646cb463d7b7bcbull, 0x4b1ffc1fb7a8a8fcull, 0xda61d6610c6d6dd6ull, 0x584e3a4e622c2c3aull
      },
      {
        0xf4a5f432c6c6a597ull, 0x9784976ff8f884ebull, 0xb099b05eeeee99c7ull, 0x8c8d8c7af6f68df7ull,
        0x170d17e8ffff0de5ull, 0xdcbddc0ad6d6bdb7ull, 0xc8b1c816dedeb1a7ull, 0xfc54fc6d91915439ull,
        0xf050f090606050c0ull, 0x0503050702020304ull, 0xe0a9e02ececea987ull, 0x877d87d156567dacull,
        0x2b192bcce7e719d5ull, 0xa662a613b5b56271ull, 0x31e6317c4d4de69aull, 0xb59ab559ecec9ac3ull,
        0xcf45cf408f8f4505ull, 0xbc9dbca31f1f9d3eull, 0xc040c04989894009ull, 0x92879268fafa87efull,
        0x3f153fd0efef15c5ull, 0x26eb2694b2b2eb7full, 0x40c940ce8e8ec907ull, 0x1d0b1de6fbfb0bedull,
        0x2fec2f6e4141ec82ull, 0xa967a91ab3b3677dull, 0x1cfd1c435f5ffdbeull, 0x25ea25604545ea8aull,
        0xdabfdaf92323bf46ull, 0x02f702515353f7a6ull, 0xa196a145e4e496d3ull, 0xed5bed769b9b5b2dull,
        0x5dc25d287575c2eaull, 0x241c24c5e1e11cd9ull, 0xe9aee9d43d3dae7aull, 0xbe6abef24c4c6a98ull,
        0xee5aee826c6c5ad8ull, 0xc341c3bd7e7e41fcull, 0x060206f3f5f502f1ull, 0xd14fd15283834f1dull,
        0xe45ce48c68685cd0ull, 0x07f407565151f4a2ull, 0x5c345c8dd1d134b9ull, 0x180818e1f9f908e9ull,
        0xae93ae4ce2e293dfull, 0x9573953eabab734dull, 0xf553f597626253c4ull, 0x413f416b2a2a3f54ull,
        0x140c141c08080c10ull, 0xf652f66395955231ull, 0xaf65afe94646658cull, 0xe25ee27f9d9d5e21ull,
        0x7828784830302860ull, 0xf8a1f8cf3737a16eull, 0x110f111b0a0a0f14ull, 0xc4b5c4eb2f2fb55eull,
        0x1b091b150e0e091cull, 0x5a365a7e24243648ull, 0xb69bb6ad1b1b9b36ull, 0x473d4798dfdf3da5ull,
        0x6a266aa7cdcd2681ull, 0xbb69bbf54e4e699cull, 0x4ccd4c337f7fcdfeull, 0xba9fba50eaea9fcfull,
        0x2d1b2d3f12121b24ull, 0xb99eb9a41d1d9e3aull, 0x9c749cc4585874b0ull, 0x722e724634342e68ull,
        0x772d774136362d6cull, 0xcdb2cd11dcdcb2a3ull, 0x29ee299db4b4ee73ull, 0x16fb164d5b5bfbb6ull,
        0x01f601a5a4a4f653ull, 0xd74dd7a176764decull, 0xa361a314b7b76175ull, 0x49ce49347d7dcefaull,
        0x8d7b8ddf52527ba4ull, 0x423e429fdddd3ea1ull, 0x937193cd5e5e71bcull, 0xa297a2b113139726ull,
        0x04f504a2a6a6f557ull, 0xb868b801b9b96869ull, 0x0000000000000000ull, 0x742c74b5c1c12c99ull,
        0xa060a0e040406080ull, 0x211f21c2e3e31fddull, 0x43c8433a7979c8f2ull, 0x2ced2c9ab6b6ed77ull,
        0xd9bed90dd4d4beb3ull, 0xca46ca478d8d4601ull, 0x70d970176767d9ceull, 0xdd4bddaf72724be4ull,
        0x79de79ed9494de33ull, 0x67d467ff9898d42bull, 0x23e82393b0b0e87bull, 0xde4ade5b85854a11ull,
        0xbd6bbd06bbbb6b6dull, 0x7e2a7ebbc5c52a91ull, 0x34e5347b4f4fe59eull, 0x3a163ad7eded16c1ull,
        0x54c554d28686c517ull, 0x62d762f89a9ad72full, 0xff55ff99666655ccull, 0xa794a7b611119422ull,
        0x4acf4ac08a8acf0full, 0x301030d9e9e910c9ull, 0x0a060a0e04040608ull, 0x98819866fefe81e7ull,
        0x0bf00baba0a0f05bull, 0xcc44ccb4787844f0ull, 0xd5bad5f02525ba4aull, 0x3ee33e754b4be396ull,
        0x0ef30eaca2a2f35full, 0x19fe19445d5dfebaull, 0x5bc05bdb8080c01bull, 0x858a858005058a0aull,
        0xecadecd33f3fad7eull, 0xdfbcdffe2121bc42ull, 0xd848d8a8707048e0ull, 0x0c040cfdf1f104f9ull,
        0x7adf7a196363dfc6ull, 0x58c1582f7777c1eeull, 0x9f759f30afaf7545ull, 0xa563a5e742426384ull,
        0x5030507020203040ull, 0x2e1a2ecbe5e51ad1ull, 0x120e12effdfd0ee1ull, 0xb76db708bfbf6d65ull,
        0xd44cd45581814c19ull, 0x3c143c2418181430ull, 0x5f355f792626354cull, 0x712f71b2c3c32f9dull,
        0x38e13886bebee167ull, 0xfda2fdc83535a26aull, 0x4fcc4fc78888cc0bull, 0x4b394b652e2e395cull,
        0xf957f96a9393573dull, 0x0df20d585555f2aaull, 0x9d829d61fcfc82e3ull, 0xc947c9b37a7a47f4ull,
        0xefacef27c8c8ac8bull, 0x32e73288babae76full, 0x7d2b7d4f32322b64ull, 0xa495a442e6e695d7ull,
        0xfba0fb3bc0c0a09bull, 0xb398b3aa19199832ull, 0x68d168f69e9ed127ull, 0x817f8122a3a37f5dull,
        0xaa66aaee44446688ull, 0x827e82d654547ea8ull, 0xe6abe6dd3b3bab76ull, 0x9e839e950b0b8316ull,
        0x45ca45c98c8cca03ull, 0x7b297bbcc7c72995ull, 0x6ed36e056b6bd3d6ull, 0x443c446c28283c50ull,
        0x8b798b2ca7a77955ull, 0x3de23d81bcbce263ull, 0x271d273116161d2cull, 0x9a769a37adad7641ull,
        0x4d3b4d96dbdb3badull, 0xfa56fa9e646456c8ull, 0xd24ed2a674744ee8ull, 0x221e223614141e28ull,
        0x76db76e49292db3full, 0x1e0a1e120c0c0a18ull, 0xb46cb4fc48486c90ull, 0x37e4378fb8b8e46bull,
        0xe75de7789f9f5d25ull, 0xb26eb20fbdbd6e61ull, 0x2aef2a694343ef86ull, 0xf1a6f135c4c4a693ull,
        0xe3a8e3da3939a872ull, 0xf7a4f7c63131a462ull, 0x5937598ad3d337bdull, 0x868b8674f2f28bffull,
        0x56325683d5d532b1ull, 0xc543c54e8b8b430dull, 0xeb59eb856e6e59dcull, 0xc2b7c218dadab7afull,
        0x8f8c8f8e01018c02ull, 0xac64ac1db1b16479ull, 0x6dd26df19c9cd223ull, 0x3be03b724949e092ull,
        0xc7b4c71fd8d8b4abull, 0x15fa15b9acacfa43ull, 0x090709faf3f307fdull, 0x6f256fa0cfcf2585ull,
        0xeaafea20cacaaf8full, 0x898e897df4f48ef3ull, 0x20e920674747e98eull, 0x2818283810101820ull,
        0x64d5640b6f6fd5deull, 0x83888373f0f088fbull, 0xb16fb1fb4a4a6f94ull, 0x967296ca5c5c72b8ull,
        0x6c246c5438382470ull, 0x08f1085f5757f1aeull, 0x52c752217373c7e6ull, 0xf351f36497975135ull,
        0x652365aecbcb238dull, 0x847c8425a1a17c59ull, 0xbf9cbf57e8e89ccbull, 0x6321635d3e3e217cull,
        0x7cdd7cea9696dd37ull, 0x7fdc7f1e6161dcc2ull, 0x9186919c0d0d861aull, 0x9485949b0f0f851eull,
        0xab90ab4be0e090dbull, 0xc642c6ba7c7c42f8ull, 0x57c457267171c4e2ull, 0xe5aae529ccccaa83ull,
        0x73d873e39090d83bull, 0x0f050f090606050cull, 0x030103f4f7f701f5ull, 0x3612362a1c1c1238ull,
        0xfea3fe3cc2c2a39full, 0xe15fe18b6a6a5fd4ull, 0x10f910beaeaef947ull, 0x6bd06b026969d0d2ull,
        0xa891a8bf1717912eull, 0xe858e87199995829ull, 0x692769533a3a2774ull, 0xd0b9d0f72727b94eull,
        0x48384891d9d938a9ull, 0x351335deebeb13cdull, 0xceb3cee52b2bb356ull, 0x5533557722223344ull,
        0xd6bbd604d2d2bbbfull, 0x90709039a9a97049ull, 0x808980870707890eull, 0xf2a7f2c13333a766ull,
        0xc1b6c1ec2d2db65aull, 0x6622665a3c3c2278ull, 0xad92adb81515922aull, 0x602060a9c9c92089ull,
        0xdb49db5c87874915ull, 0x1aff1ab0aaaaff4full, 0x887888d8505078a0ull, 0x8e7a8e2ba5a57a51ull,
        0x8a8f8a8903038f06ull, 0x13f8134a5959f8b2ull, 0x9b809b9209098012ull, 0x391739231a1a1734ull,
        0x75da75106565dacaull, 0x53315384d7d731b5ull, 0x51c651d58484c613ull, 0xd3b8d303d0d0b8bbull,
        0x5ec35edc8282c31full, 0xcbb0cbe22929b052ull, 0x997799c35a5a77b4ull, 0x3311332d1e1e113cull,
        0x46cb463d7b7bcbf6ull, 0x1ffc1fb7a8a8fc4bull, 0x61d6610c6d6dd6daull, 0x4e3a4e622c2c3a58ull
      },
      {
        0xa5f432c6c6a597f4ull, 0x84976ff8f884eb97ull, 0x99b05eeeee99c7b0ull, 0x8d8c7af6f68df78cull,
        0x0d17e8ffff0de517ull, 0xbddc0ad6d6bdb7dcull, 0xb1c816dedeb1a7c8ull, 0x54fc6d91915439fcull,
        0x50f090606050c0f0ull, 0x0305070202030405ull, 0xa9e02ececea987e0ull, 0x7d87d156567dac87ull,
        0x192bcce7e719d52bull, 0x62a613b5b56271a6ull, 0xe6317c4d4de69a31ull, 0x9ab559ecec9ac3b5ull,
        0x45cf408f8f4505cfull, 0x9dbca31f1f9d3ebcull, 0x40c04989894009c0ull, 0x879268fafa87ef92ull,
        0x153fd0efef15c53full, 0xeb2694b2b2eb7f26ull, 0xc940ce8e8ec90740ull, 0x0b1de6fbfb0bed1dull,
        0xec2f6e4141ec822full, 0x67a91ab3b3677da9ull, 0xfd1c435f5ffdbe1cull, 0xea25604545ea8a25ull,
        0xbfdaf92323bf46daull, 0xf702515353f7a602ull, 0x96a145e4e496d3a1ull, 0x5bed769b9b5b2dedull,
        0xc25d287575c2ea5dull, 0x1c24c5e1e11cd924ull, 0xaee9d43d3dae7ae9ull, 0x6abef24c4c6a98beull,
        0x5aee826c6c5ad8eeull, 0x41c3bd7e7e41fcc3ull, 0x0206f3f5f502f106ull, 0x4fd15283834f1dd1ull,
        0x5ce48c68685cd0e4ull, 0xf407565151f4a207ull, 0x345c8dd1d134b95cull, 0x0818e1f9f908e918ull,
        0x93ae4ce2e293dfaeull, 0x73953eabab734d95ull, 0x53f597626253c4f5ull, 0x3f416b2a2a3f5441ull,
        0x0c141c08080c1014ull, 0x52f66395955231f6ull, 0x65afe94646658cafull, 0x5ee27f9d9d5e21e2ull,
        0x2878483030286078ull, 0xa1f8cf3737a16ef8ull, 0x0f111b0a0a0f1411ull, 0xb5c4eb2f2fb55ec4ull,
        0x091b150e0e091c1bull, 0x365a7e242436485aull, 0x9bb6ad1b1b9b36b6ull, 0x3d4798dfdf3da547ull,
        0x266aa7cdcd26816aull, 0x69bbf54e4e699cbbull, 0xcd4c337f7fcdfe4cull, 0x9fba50eaea9fcfbaull,
        0x1b2d3f12121b242dull, 0x9eb9a41d1d9e3ab9ull, 0x749cc4585874b09cull, 0x2e724634342e6872ull,
        0x2d774136362d6c77ull, 0xb2cd11dcdcb2a3cdull, 0xee299db4b4ee7329ull, 0xfb164d5b5bfbb616ull,
        0xf601a5a4a4f65301ull, 0x4dd7a176764decd7ull, 0x61a314b7b76175a3ull, 0xce49347d7dcefa49ull,
        0x7b8ddf52527ba48dull, 0x3e429fdddd3ea142ull, 0x7193cd5e5e71bc93ull, 0x97a2b113139726a2ull,
        0xf504a2a6a6f55704ull, 0x68b801b9b96869b8ull, 0x0000000000000000ull, 0x2c74b5c1c12c9974ull,
        0x60a0e040406080a0ull, 0x1f21c2e3e31fdd21ull, 0xc8433a7979c8f243ull, 0xed2c9ab6b6ed772cull,
        0xbed90dd4d4beb3d9ull, 0x46ca478d8d4601caull, 0xd970176767d9ce70ull, 0x4bddaf72724be4ddull,
        0xde79ed9494de3379ull, 0xd467ff9898d42b67ull, 0xe82393b0b0e87b23ull, 0x4ade5b85854a11deull,
        0x6bbd06bbbb6b6dbdull, 0x2a7ebbc5c52a917eull, 0xe5347b4f4fe59e34ull, 0x163ad7eded16c13aull,
        0xc554d28686c51754ull, 0xd762f89a9ad72f62ull, 0x55ff99666655ccffull, 0x94a7b611119422a7ull,
        0xcf4ac08a8acf0f4aull, 0x1030d9e9e910c930ull, 0x060a0e040406080aull, 0x819866fefe81e798ull,
        0xf00baba0a0f05b0bull, 0x44ccb4787844f0ccull, 0xbad5f02525ba4ad5ull, 0xe33e754b4be3963eull,
        0xf30eaca2a2f35f0eull, 0xfe19445d5dfeba19ull, 0xc05bdb8080c01b5bull, 0x8a858005058a0a85ull,
        0xadecd33f3fad7eecull, 0xbcdffe2121bc42dfull, 0x48d8a8707048e0d8ull, 0x040cfdf1f104f90cull,
        0xdf7a196363dfc67aull, 0xc1582f7777c1ee58ull, 0x759f30afaf75459full, 0x63a5e742426384a5ull,
        0x3050702020304050ull, 0x1a2ecbe5e51ad12eull, 0x0e12effdfd0ee112ull, 0x6db708bfbf6d65b7ull,
        0x4cd45581814c19d4ull, 0x143c24181814303cull, 0x355f792626354c5full, 0x2f71b2c3c32f9d71ull,
        0xe13886bebee16738ull, 0xa2fdc83535a26afdull, 0xcc4fc78888cc0b4full, 0x394b652e2e395c4bull,
        0x57f96a9393573df9ull, 0xf20d585555f2aa0dull, 0x829d61fcfc82e39dull, 0x47c9b37a7a47f4c9ull,
        0xacef27c8c8ac8befull, 0xe73288babae76f32ull, 0x2b7d4f32322b647dull, 0x95a442e6e695d7a4ull,
        0xa0fb3bc0c0a09bfbull, 0x98b3aa19199832b3ull, 0xd168f69e9ed12768ull, 0x7f8122a3a37f5d81ull,
        0x66aaee44446688aaull, 0x7e82d654547ea882ull, 0xabe6dd3b3bab76e6ull, 0x839e950b0b83169eull,
        0xca45c98c8cca0345ull, 0x297bbcc7c729957bull, 0xd36e056b6bd3d66eull, 0x3c446c28283c5044ull,
        0x798b2ca7a779558bull, 0xe23d81bcbce2633dull, 0x1d273116161d2c27ull, 0x769a37adad76419aull,
        0x3b4d96dbdb3bad4dull, 0x56fa9e646456c8faull, 0x4ed2a674744ee8d2ull, 0x1e223614141e2822ull,
        0xdb76e49292db3f76ull, 0x0a1e120c0c0a181eull, 0x6cb4fc48486c90b4ull, 0xe4378fb8b8e46b37ull,
        0x5de7789f9f5d25e7ull, 0x6eb20fbdbd6e61b2ull, 0xef2a694343ef862aull, 0xa6f135c4c4a693f1ull,
        0xa8e3da3939a872e3ull, 0xa4f7c63131a462f7ull, 0x37598ad3d337bd59ull, 0x8b8674f2f28bff86ull,
        0x325683d5d532b156ull, 0x43c54e8b8b430dc5ull, 0x59eb856e6e59dcebull, 0xb7c218dadab7afc2ull,
        0x8c8f8e01018c028full, 0x64ac1db1b16479acull, 0xd26df19c9cd2236dull, 0xe03b724949e0923bull,
        0xb4c71fd8d8b4abc7ull, 0xfa15b9acacfa4315ull, 0x0709faf3f307fd09ull, 0x256fa0cfcf25856full,
        0xafea20cacaaf8feaull, 0x8e897df4f48ef389ull, 0xe920674747e98e20ull, 0x1828381010182028ull,
        0xd5640b6f6fd5de64ull, 0x888373f0f088fb83ull, 0x6fb1fb4a4a6f94b1ull, 0x7296ca5c5c72b896ull,
        0x246c54383824706cull, 0xf1085f5757f1ae08ull, 0xc752217373c7e652ull, 0x51f36497975135f3ull,
        0x2365aecbcb238d65ull, 0x7c8425a1a17c5984ull, 0x9cbf57e8e89ccbbfull, 0x21635d3e3e217c63ull,
        0xdd7cea9696dd377cull, 0xdc7f1e6161dcc27full, 0x86919c0d0d861a91ull, 0x85949b0f0f851e94ull,
        0x90ab4be0e090dbabull, 0x42c6ba7c7c42f8c6ull, 0xc457267171c4e257ull, 0xaae529ccccaa83e5ull,
        0xd873e39090d83b73ull, 0x050f090606050c0full, 0x0103f4f7f701f503ull, 0x12362a1c1c123836ull,
        0xa3fe3cc2c2a39ffeull, 0x5fe18b6a6a5fd4e1ull, 0xf910beaeaef94710ull, 0xd06b026969d0d26bull,
        0x91a8bf1717912ea8ull, 0x58e87199995829e8ull, 0x2769533a3a277469ull, 0xb9d0f72727b94ed0ull,
        0x384891d9d938a948ull, 0x1335deebeb13cd35ull, 0xb3cee52b2bb356ceull, 0x3355772222334455ull,
        0xbbd604d2d2bbbfd6ull, 0x709039a9a9704990ull, 0x8980870707890e80ull, 0xa7f2c13333a766f2ull,
        0xb6c1ec2d2db65ac1ull, 0x22665a3c3c227866ull, 0x92adb81515922aadull, 0x2060a9c9c9208960ull,
        0x49db5c87874915dbull, 0xff1ab0aaaaff4f1aull, 0x7888d8505078a088ull, 0x7a8e2ba5a57a518eull,
        0x8f8a8903038f068aull, 0xf8134a5959f8b213ull, 0x809b92090980129bull, 0x1739231a1a173439ull,
        0xda75106565daca75ull, 0x315384d7d731b553ull, 0xc651d58484c61351ull, 0xb8d303d0d0b8bbd3ull,
        0xc35edc8282c31f5eull, 0xb0cbe22929b052cbull, 0x7799c35a5a77b499ull, 0x11332d1e1e113c33ull,
        0xcb463d7b7bcbf646ull, 0xfc1fb7a8a8fc4b1full, 0xd6610c6d6dd6da61ull, 0x3a4e622c2c3a584eull
      },
      {
        0xf432c6c6a597f4a5ull, 0x976ff8f884eb9784ull, 0xb05eeeee99c7b099ull, 0x8c7af6f68df78c8dull,
        0x17e8ffff0de5170dull, 0xdc0ad6d6bdb7dcbdull, 0xc816dedeb1a7c8b1ull, 0xfc6d91915439fc54ull,
        0xf090606050c0f050ull, 0x0507020203040503ull, 0xe02ececea987e0a9ull, 0x87d156567dac877dull,
        0x2bcce7e719d52b19ull, 0xa613b5b56271a662ull, 0x317c4d4de69a31e6ull, 0xb559ecec9ac3b59aull,
        0xcf408f8f4505cf45ull, 0xbca31f1f9d3ebc9dull, 0xc04989894009c040ull, 0x9268fafa87ef9287ull,
        0x3fd0efef15c53f15ull, 0x2694b2b2eb7f26ebull, 0x40ce8e8ec90740c9ull, 0x1de6fbfb0bed1d0bull,
        0x2f6e4141ec822fecull, 0xa91ab3b3677da967ull, 0x1c435f5ffdbe1cfdull, 0x25604545ea8a25eaull,
        0xdaf92323bf46dabfull, 0x02515353f7a602f7ull, 0xa145e4e496d3a196ull, 0xed769b9b5b2ded5bull,
        0x5d287575c2ea5dc2ull, 0x24c5e1e11cd9241cull, 0xe9d43d3dae7ae9aeull, 0xbef24c4c6a98be6aull,
        0xee826c6c5ad8ee5aull, 0xc3bd7e7e41fcc341ull, 0x06f3f5f502f10602ull, 0xd15283834f1dd14full,
        0xe48c68685cd0e45cull, 0x07565151f4a207f4ull, 0x5c8dd1d134b95c34ull, 0x18e1f9f908e91808ull,
        0xae4ce2e293dfae93ull, 0x953eabab734d9573ull, 0xf597626253c4f553ull, 0x416b2a2a3f54413full,
        0x141c08080c10140cull, 0xf66395955231f652ull, 0xafe94646658caf65ull, 0xe27f9d9d5e21e25eull,
        0x7848303028607828ull, 0xf8cf3737a16ef8a1ull, 0x111b0a0a0f14110full, 0xc4eb2f2fb55ec4b5ull,
        0x1b150e0e091c1b09ull, 0x5a7e242436485a36ull, 0xb6ad1b1b9b36b69bull, 0x4798dfdf3da5473dull,
        0x6aa7cdcd26816a26ull, 0xbbf54e4e699cbb69ull, 0x4c337f7fcdfe4ccdull, 0xba50eaea9fcfba9full,
        0x2d3f12121b242d1bull, 0xb9a41d1d9e3ab99eull, 0x9cc4585874b09c74ull, 0x724634342e68722eull,
        0x774136362d6c772dull, 0xcd11dcdcb2a3cdb2ull, 0x299db4b4ee7329eeull, 0x164d5b5bfbb616fbull,
        0x01a5a4a4f65301f6ull, 0xd7a176764decd74dull, 0xa314b7b76175a361ull, 0x49347d7dcefa49ceull,
        0x8ddf52527ba48d7bull, 0x429fdddd3ea1423eull, 0x93cd5e5e71bc9371ull, 0xa2b113139726a297ull,
        0x04a2a6a6f55704f5ull, 0xb801b9b96869b868ull, 0x0000000000000000ull, 0x74b5c1c12c99742cull,
        0xa0e040406080a060ull, 0x21c2e3e31fdd211full, 0x433a7979c8f243c8ull, 0x2c9ab6b6ed772cedull,
        0xd90dd4d4beb3d9beull, 0xca478d8d4601ca46ull, 0x70176767d9ce70d9ull, 0xddaf72724be4dd4bull,
        0x79ed9494de3379deull, 0x67ff9898d42b67d4ull, 0x2393b0b0e87b23e8ull, 0xde5b85854a11de4aull,
        0xbd06bbbb6b6dbd6bull, 0x7ebbc5c52a917e2aull, 0x347b4f4fe59e34e5ull, 0x3ad7eded16c13a16ull,
        0x54d28686c51754c5ull, 0x62f89a9ad72f62d7ull, 0xff99666655ccff55ull, 0xa7b611119422a794ull,
        0x4ac08a8acf0f4acfull, 0x30d9e9e910c93010ull, 0x0a0e040406080a06ull, 0x9866fefe81e79881ull,
        0x0baba0a0f05b0bf0ull, 0xccb4787844f0cc44ull, 0xd5f02525ba4ad5baull, 0x3e754b4be3963ee3ull,
        0x0eaca2a2f35f0ef3ull, 0x19445d5dfeba19feull, 0x5bdb8080c01b5bc0ull, 0x858005058a0a858aull,
        0xecd33f3fad7eecadull, 0xdffe2121bc42dfbcull, 0xd8a8707048e0d848ull, 0x0cfdf1f104f90c04ull,
        0x7a196363dfc67adfull, 0x582f7777c1ee58c1ull, 0x9f30afaf75459f75ull, 0xa5e742426384a563ull,
        0x5070202030405030ull, 0x2ecbe5e51ad12e1aull, 0x12effdfd0ee1120eull, 0xb708bfbf6d65b76dull,
        0xd45581814c19d44cull, 0x3c24181814303c14ull, 0x5f792626354c5f35ull, 0x71b2c3c32f9d712full,
        0x3886bebee16738e1ull, 0xfdc83535a26afda2ull, 0x4fc78888cc0b4fccull, 0x4b652e2e395c4b39ull,
        0xf96a9393573df957ull, 0x0d585555f2aa0df2ull, 0x9d61fcfc82e39d82ull, 0xc9b37a7a47f4c947ull,
        0xef27c8c8ac8befacull, 0x3288babae76f32e7ull, 0x7d4f32322b647d2bull, 0xa442e6e695d7a495ull,
        0xfb3bc0c0a09bfba0ull, 0xb3aa19199832b398ull, 0x68f69e9ed12768d1ull, 0x8122a3a37f5d817full,
        0xaaee44446688aa66ull, 0x82d654547ea8827eull, 0xe6dd3b3bab76e6abull, 0x9e950b0b83169e83ull,
        0x45c98c8cca0345caull, 0x7bbcc7c729957b29ull, 0x6e056b6bd3d66ed3ull, 0x446c28283c50443cull,
        0x8b2ca7a779558b79ull, 0x3d81bcbce2633de2ull, 0x273116161d2c271dull, 0x9a37adad76419a76ull,
        0x4d96dbdb3bad4d3bull, 0xfa9e646456c8fa56ull, 0xd2a674744ee8d24eull, 0x223614141e28221eull,
        0x76e49292db3f76dbull, 0x1e120c0c0a181e0aull, 0xb4fc48486c90b46cull, 0x378fb8b8e46b37e4ull,
        0xe7789f9f5d25e75dull, 0xb20fbdbd6e61b26eull, 0x2a694343ef862aefull, 0xf135c4c4a693f1a6ull,
        0xe3da3939a872e3a8ull, 0xf7c63131a462f7a4ull, 0x598ad3d337bd5937ull, 0x8674f2f28bff868bull,
        0x5683d5d532b15632ull, 0xc54e8b8b430dc543ull, 0xeb856e6e59dceb59ull, 0xc218dadab7afc2b7ull,
        0x8f8e01018c028f8cull, 0xac1db1b16479ac64ull, 0x6df19c9cd2236dd2ull, 0x3b724949e0923be0ull,
        0xc71fd8d8b4abc7b4ull, 0x15b9acacfa4315faull, 0x09faf3f307fd0907ull, 0x6fa0cfcf25856f25ull,
        0xea20cacaaf8feaafull, 0x897df4f48ef3898eull, 0x20674747e98e20e9ull, 0x2838101018202818ull,
        0x640b6f6fd5de64d5ull, 0x8373f0f088fb8388ull, 0xb1fb4a4a6f94b16full, 0x96ca5c5c72b89672ull,
        0x6c54383824706c24ull, 0x085f5757f1ae08f1ull, 0x52217373c7e652c7ull, 0xf36497975135f351ull,
        0x65aecbcb238d6523ull, 0x8425a1a17c59847cull, 0xbf57e8e89ccbbf9cull, 0x635d3e3e217c6321ull,
        0x7cea9696dd377cddull, 0x7f1e6161dcc27fdcull, 0x919c0d0d861a9186ull, 0x949b0f0f851e9485ull,
        0xab4be0e090dbab90ull, 0xc6ba7c7c42f8c642ull, 0x57267171c4e257c4ull, 0xe529ccccaa83e5aaull,
        0x73e39090d83b73d8ull, 0x0f090606050c0f05ull, 0x03f4f7f701f50301ull, 0x362a1c1c12383612ull,
        0xfe3cc2c2a39ffea3ull, 0xe18b6a6a5fd4e15full, 0x10beaeaef94710f9ull, 0x6b026969d0d26bd0ull,
        0xa8bf1717912ea891ull, 0xe87199995829e858ull, 0x69533a3a27746927ull, 0xd0f72727b94ed0b9ull,
        0x4891d9d938a94838ull, 0x35deebeb13cd3513ull, 0xcee52b2bb356ceb3ull, 0x5577222233445533ull,
        0xd604d2d2bbbfd6bbull, 0x9039a9a970499070ull, 0x80870707890e8089ull, 0xf2c13333a766f2a7ull,
        0xc1ec2d2db65ac1b6ull, 0x665a3c3c22786622ull, 0xadb81515922aad92ull, 0x60a9c9c920896020ull,
        0xdb5c87874915db49ull, 0x1ab0aaaaff4f1affull, 0x88d8505078a08878ull, 0x8e2ba5a57a518e7aull,
        0x8a8903038f068a8full, 0x134a5959f8b213f8ull, 0x9b92090980129b80ull, 0x39231a1a17343917ull,
        0x75106565daca75daull, 0x5384d7d731b55331ull, 0x51d58484c61351c6ull, 0xd303d0d0b8bbd3b8ull,
        0x5edc8282c31f5ec3ull, 0xcbe22929b052cbb0ull, 0x99c35a5a77b49977ull, 0x332d1e1e113c3311ull,
        0x463d7b7bcbf646cbull, 0x1fb7a8a8fc4b1ffcull, 0x610c6d6dd6da61d6ull, 0x4e622c2c3a584e3aull
      },
      {
        0x32c6c6a597f4a5f4ull, 0x6ff8f884eb978497ull, 0x5eeeee99c7b099b0ull, 0x7af6f68df78c8d8cull,
        0xe8ffff0de5170d17ull, 0x0ad6d6bdb7dcbddcull, 0x16dedeb1a7c8b1c8ull, 0x6d91915439fc54fcull,
        0x90606050c0f050f0ull, 0x0702020304050305ull, 0x2ececea987e0a9e0ull, 0xd156567dac877d87ull,
        0xcce7e719d52b192bull, 0x13b5b56271a662a6ull, 0x7c4d4de69a31e631ull, 0x59ecec9ac3b59ab5ull,
        0x408f8f4505cf45cfull, 0xa31f1f9d3ebc9dbcull, 0x4989894009c040c0ull, 0x68fafa87ef928792ull,
        0xd0efef15c53f153full, 0x94b2b2eb7f26eb26ull, 0xce8e8ec90740c940ull, 0xe6fbfb0bed1d0b1dull,
        0x6e4141ec822fec2full, 0x1ab3b3677da967a9ull, 0x435f5ffdbe1cfd1cull, 0x604545ea8a25ea25ull,
        0xf92323bf46dabfdaull, 0x515353f7a602f702ull, 0x45e4e496d3a196a1ull, 0x769b9b5b2ded5bedull,
        0x287575c2ea5dc25dull, 0xc5e1e11cd9241c24ull, 0xd43d3dae7ae9aee9ull, 0xf24c4c6a98be6abeull,
        0x826c6c5ad8ee5aeeull, 0xbd7e7e41fcc341c3ull, 0xf3f5f502f1060206ull, 0x5283834f1dd14fd1ull,
        0x8c68685cd0e45ce4ull, 0x565151f4a207f407ull, 0x8dd1d134b95c345cull, 0xe1f9f908e9180818ull,
        0x4ce2e293dfae93aeull, 0x3eabab734d957395ull, 0x97626253c4f553f5ull, 0x6b2a2a3f54413f41ull,
        0x1c08080c10140c14ull, 0x6395955231f652f6ull, 0xe94646658caf65afull, 0x7f9d9d5e21e25ee2ull,
        0x4830302860782878ull, 0xcf3737a16ef8a1f8ull, 0x1b0a0a0f14110f11ull, 0xeb2f2fb55ec4b5c4ull,
        0x150e0e091c1b091bull, 0x7e242436485a365aull, 0xad1b1b9b36b69bb6ull, 0x98dfdf3da5473d47ull,
        0xa7cdcd26816a266aull, 0xf54e4e699cbb69bbull, 0x337f7fcdfe4ccd4cull, 0x50eaea9fcfba9fbaull,
        0x3f12121b242d1b2dull, 0xa41d1d9e3ab99eb9ull, 0xc4585874b09c749cull, 0x4634342e68722e72ull,
        0x4136362d6c772d77ull, 0x11dcdcb2a3cdb2cdull, 0x9db4b4ee7329ee29ull, 0x4d5b5bfbb616fb16ull,
        0xa5a4a4f65301f601ull, 0xa176764decd74dd7ull, 0x14b7b76175a361a3ull, 0x347d7dcefa49ce49ull,
        0xdf52527ba48d7b8dull, 0x9fdddd3ea1423e42ull, 0xcd5e5e71bc937193ull, 0xb113139726a297a2ull,
        0xa2a6a6f55704f504ull, 0x01b9b96869b868b8ull, 0x0000000000000000ull, 0xb5c1c12c99742c74ull,
        0xe040406080a060a0ull, 0xc2e3e31fdd211f21ull, 0x3a7979c8f243c843ull, 0x9ab6b6ed772ced2cull,
        0x0dd4d4beb3d9bed9ull, 0x478d8d4601ca46caull, 0x176767d9ce70d970ull, 0xaf72724be4dd4bddull,
        0xed9494de3379de79ull, 0xff9898d42b67d467ull, 0x93b0b0e87b23e823ull, 0x5b85854a11de4adeull,
        0x06bbbb6b6dbd6bbdull, 0xbbc5c52a917e2a7eull, 0x7b4f4fe59e34e534ull, 0xd7eded16c13a163aull,
        0xd28686c51754c554ull, 0xf89a9ad72f62d762ull, 0x99666655ccff55ffull, 0xb611119422a794a7ull,
        0xc08a8acf0f4acf4aull, 0xd9e9e910c9301030ull, 0x0e040406080a060aull, 0x66fefe81e7988198ull,
        0xaba0a0f05b0bf00bull, 0xb4787844f0cc44ccull, 0xf02525ba4ad5bad5ull, 0x754b4be3963ee33eull,
        0xaca2a2f35f0ef30eull, 0x445d5dfeba19fe19ull, 0xdb8080c01b5bc05bull, 0x8005058a0a858a85ull,
        0xd33f3fad7eecadecull, 0xfe2121bc42dfbcdfull, 0xa8707048e0d848d8ull, 0xfdf1f104f90c040cull,
        0x196363dfc67adf7aull, 0x2f7777c1ee58c158ull, 0x30afaf75459f759full, 0xe742426384a563a5ull,
        0x7020203040503050ull, 0xcbe5e51ad12e1a2eull, 0xeffdfd0ee1120e12ull, 0x08bfbf6d65b76db7ull,
        0x5581814c19d44cd4ull, 0x24181814303c143cull, 0x792626354c5f355full, 0xb2c3c32f9d712f71ull,
        0x86bebee16738e138ull, 0xc83535a26afda2fdull, 0xc78888cc0b4fcc4full, 0x652e2e395c4b394bull,
        0x6a9393573df957f9ull, 0x585555f2aa0df20dull, 0x61fcfc82e39d829dull, 0xb37a7a47f4c947c9ull,
        0x27c8c8ac8befacefull, 0x88babae76f32e732ull, 0x4f32322b647d2b7dull, 0x42e6e695d7a495a4ull,
        0x3bc0c0a09bfba0fbull, 0xaa19199832b398b3ull, 0xf69e9ed12768d168ull, 0x22a3a37f5d817f81ull,
        0xee44446688aa66aaull, 0xd654547ea8827e82ull, 0xdd3b3bab76e6abe6ull, 0x950b0b83169e839eull,
        0xc98c8cca0345ca45ull, 0xbcc7c729957b297bull, 0x056b6bd3d66ed36eull, 0x6c28283c50443c44ull,
        0x2ca7a779558b798bull, 0x81bcbce2633de23dull, 0x3116161d2c271d27ull, 0x37adad76419a769aull,
        0x96dbdb3bad4d3b4dull, 0x9e646456c8fa56faull, 0xa674744ee8d24ed2ull, 0x3614141e28221e22ull,
        0xe49292db3f76db76ull, 0x120c0c0a181e0a1eull, 0xfc48486c90b46cb4ull, 0x8fb8b8e46b37e437ull,
        0x789f9f5d25e75de7ull, 0x0fbdbd6e61b26eb2ull, 0x694343ef862aef2aull, 0x35c4c4a693f1a6f1ull,
        0xda3939a872e3a8e3ull, 0xc63131a462f7a4f7ull, 0x8ad3d337bd593759ull, 0x74f2f28bff868b86ull,
        0x83d5d532b1563256ull, 0x4e8b8b430dc543c5ull, 0x856e6e59dceb59ebull, 0x18dadab7afc2b7c2ull,
        0x8e01018c028f8c8full, 0x1db1b16479ac64acull, 0xf19c9cd2236dd26dull, 0x724949e0923be03bull,
        0x1fd8d8b4abc7b4c7ull, 0xb9acacfa4315fa15ull, 0xfaf3f307fd090709ull, 0xa0cfcf25856f256full,
        0x20cacaaf8feaafeaull, 0x7df4f48ef3898e89ull, 0x674747e98e20e920ull, 0x3810101820281828ull,
        0x0b6f6fd5de64d564ull, 0x73f0f088fb838883ull, 0xfb4a4a6f94b16fb1ull, 0xca5c5c72b8967296ull,
        0x54383824706c246cull, 0x5f5757f1ae08f108ull, 0x217373c7e652c752ull, 0x6497975135f351f3ull,
        0xaecbcb238d652365ull, 0x25a1a17c59847c84ull, 0x57e8e89ccbbf9cbfull, 0x5d3e3e217c632163ull,
        0xea9696dd377cdd7cull, 0x1e6161dcc27fdc7full, 0x9c0d0d861a918691ull, 0x9b0f0f851e948594ull,
        0x4be0e090dbab90abull, 0xba7c7c42f8c642c6ull, 0x267171c4e257c457ull, 0x29ccccaa83e5aae5ull,
        0xe39090d83b73d873ull, 0x090606050c0f050full, 0xf4f7f701f5030103ull, 0x2a1c1c1238361236ull,
        0x3cc2c2a39ffea3feull, 0x8b6a6a5fd4e15fe1ull, 0xbeaeaef94710f910ull, 0x026969d0d26bd06bull,
        0xbf1717912ea891a8ull, 0x7199995829e858e8ull, 0x533a3a2774692769ull, 0xf72727b94ed0b9d0ull,
        0x91d9d938a9483848ull, 0xdeebeb13cd351335ull, 0xe52b2bb356ceb3ceull, 0x7722223344553355ull,
        0x04d2d2bbbfd6bbd6ull, 0x39a9a97049907090ull, 0x870707890e808980ull, 0xc13333a766f2a7f2ull,
        0xec2d2db65ac1b6c1ull, 0x5a3c3c2278662266ull, 0xb81515922aad92adull, 0xa9c9c92089602060ull,
        0x5c87874915db49dbull, 0xb0aaaaff4f1aff1aull, 0xd8505078a0887888ull, 0x2ba5a57a518e7a8eull,
        0x8903038f068a8f8aull, 0x4a5959f8b213f813ull, 0x92090980129b809bull, 0x231a1a1734391739ull,
        0x106565daca75da75ull, 0x84d7d731b5533153ull, 0xd58484c61351c651ull, 0x03d0d0b8bbd3b8d3ull,
        0xdc8282c31f5ec35eull, 0xe22929b052cbb0cbull, 0xc35a5a77b4997799ull, 0x2d1e1e113c331133ull,
        0x3d7b7bcbf646cb46ull, 0xb7a8a8fc4b1ffc1full, 0x0c6d6dd6da61d661ull, 0x622c2c3a584e3a4eull
      },
      {
        0xc6c6a597f4a5f432ull, 0xf8f884eb9784976full, 0xeeee99c7b099b05eull, 0xf6f68df78c8d8c7aull,
        0xffff0de5170d17e8ull, 0xd6d6bdb7dcbddc0aull, 0xdedeb1a7c8b1c816ull, 0x91915439fc54fc6dull,
        0x606050c0f050f090ull, 0x0202030405030507ull, 0xcecea987e0a9e02eull, 0x56567dac877d87d1ull,
        0xe7e719d52b192bccull, 0xb5b56271a662a613ull, 0x4d4de69a31e6317cull, 0xecec9ac3b59ab559ull,
        0x8f8f4505cf45cf40ull, 0x1f1f9d3ebc9dbca3ull, 0x89894009c040c049ull, 0xfafa87ef92879268ull,
        0xefef15c53f153fd0ull, 0xb2b2eb7f26eb2694ull, 0x8e8ec90740c940ceull, 0xfbfb0bed1d0b1de6ull,
        0x4141ec822fec2f6eull, 0xb3b3677da967a91aull, 0x5f5ffdbe1cfd1c43ull, 0x4545ea8a25ea2560ull,
        0x2323bf46dabfdaf9ull, 0x5353f7a602f70251ull, 0xe4e496d3a196a145ull, 0x9b9b5b2ded5bed76ull,
        0x7575c2ea5dc25d28ull, 0xe1e11cd9241c24c5ull, 0x3d3dae7ae9aee9d4ull, 0x4c4c6a98be6abef2ull,
        0x6c6c5ad8ee5aee82ull, 0x7e7e41fcc341c3bdull, 0xf5f502f1060206f3ull, 0x83834f1dd14fd152ull,
        0x68685cd0e45ce48cull, 0x5151f4a207f40756ull, 0xd1d134b95c345c8dull, 0xf9f908e9180818e1ull,
        0xe2e293dfae93ae4cull, 0xabab734d9573953eull, 0x626253c4f553f597ull, 0x2a2a3f54413f416bull,
        0x08080c10140c141cull, 0x95955231f652f663ull, 0x4646658caf65afe9ull, 0x9d9d5e21e25ee27full,
        0x3030286078287848ull, 0x3737a16ef8a1f8cfull, 0x0a0a0f14110f111bull, 0x2f2fb55ec4b5c4ebull,
        0x0e0e091c1b091b15ull, 0x242436485a365a7eull, 0x1b1b9b36b69bb6adull, 0xdfdf3da5473d4798ull,
        0xcdcd26816a266aa7ull, 0x4e4e699cbb69bbf5ull, 0x7f7fcdfe4ccd4c33ull, 0xeaea9fcfba9fba50ull,
        0x12121b242d1b2d3full, 0x1d1d9e3ab99eb9a4ull, 0x585874b09c749cc4ull, 0x34342e68722e7246ull,
        0x36362d6c772d7741ull, 0xdcdcb2a3cdb2cd11ull, 0xb4b4ee7329ee299dull, 0x5b5bfbb616fb164dull,
        0xa4a4f65301f601a5ull, 0x76764decd74dd7a1ull, 0xb7b76175a361a314ull, 0x7d7dcefa49ce4934ull,
        0x52527ba48d7b8ddfull, 0xdddd3ea1423e429full, 0x5e5e71bc937193cdull, 0x13139726a297a2b1ull,
        0xa6a6f55704f504a2ull, 0xb9b96869b868b801ull, 0x0000000000000000ull, 0xc1c12c99742c74b5ull,
        0x40406080a060a0e0ull, 0xe3e31fdd211f21c2ull, 0x7979c8f243c8433aull, 0xb6b6ed772ced2c9aull,
        0xd4d4beb3d9bed90dull, 0x8d8d4601ca46ca47ull, 0x6767d9ce70d97017ull, 0x72724be4dd4bddafull,
        0x9494de3379de79edull, 0x9898d42b67d467ffull, 0xb0b0e87b23e82393ull, 0x85854a11de4ade5bull,
        0xbbbb6b6dbd6bbd06ull, 0xc5c52a917e2a7ebbull, 0x4f4fe59e34e5347bull, 0xeded16c13a163ad7ull,
        0x8686c51754c554d2ull, 0x9a9ad72f62d762f8ull, 0x666655ccff55ff99ull, 0x11119422a794a7b6ull,
        0x8a8acf0f4acf4ac0ull, 0xe9e910c9301030d9ull, 0x040406080a060a0eull, 0xfefe81e798819866ull,
        0xa0a0f05b0bf00babull, 0x787844f0cc44ccb4ull, 0x2525ba4ad5bad5f0ull, 0x4b4be3963ee33e75ull,
        0xa2a2f35f0ef30eacull, 0x5d5dfeba19fe1944ull, 0x8080c01b5bc05bdbull, 0x05058a0a858a8580ull,
        0x3f3fad7eecadecd3ull, 0x2121bc42dfbcdffeull, 0x707048e0d848d8a8ull, 0xf1f104f90c040cfdull,
        0x6363dfc67adf7a19ull, 0x7777c1ee58c1582full, 0xafaf75459f759f30ull, 0x42426384a563a5e7ull,
        0x2020304050305070ull, 0xe5e51ad12e1a2ecbull, 0xfdfd0ee1120e12efull, 0xbfbf6d65b76db708ull,
        0x81814c19d44cd455ull, 0x181814303c143c24ull, 0x2626354c5f355f79ull, 0xc3c32f9d712f71b2ull,
        0xbebee16738e13886ull, 0x3535a26afda2fdc8ull, 0x8888cc0b4fcc4fc7ull, 0x2e2e395c4b394b65ull,
        0x9393573df957f96aull, 0x5555f2aa0df20d58ull, 0xfcfc82e39d829d61ull, 0x7a7a47f4c947c9b3ull,
        0xc8c8ac8befacef27ull, 0xbabae76f32e73288ull, 0x32322b647d2b7d4full, 0xe6e695d7a495a442ull,
        0xc0c0a09bfba0fb3bull, 0x19199832b398b3aaull, 0x9e9ed12768d168f6ull, 0xa3a37f5d817f8122ull,
        0x44446688aa66aaeeull, 0x54547ea8827e82d6ull, 0x3b3bab76e6abe6ddull, 0x0b0b83169e839e95ull,
        0x8c8cca0345ca45c9ull, 0xc7c729957b297bbcull, 0x6b6bd3d66ed36e05ull, 0x28283c50443c446cull,
        0xa7a779558b798b2cull, 0xbcbce2633de23d81ull, 0x16161d2c271d2731ull, 0xadad76419a769a37ull,
        0xdbdb3bad4d3b4d96ull, 0x646456c8fa56fa9eull, 0x74744ee8d24ed2a6ull, 0x14141e28221e2236ull,
        0x9292db3f76db76e4ull, 0x0c0c0a181e0a1e12ull, 0x48486c90b46cb4fcull, 0xb8b8e46b37e4378full,
        0x9f9f5d25e75de778ull, 0xbdbd6e61b26eb20full, 0x4343ef862aef2a69ull, 0xc4c4a693f1a6f135ull,
        0x3939a872e3a8e3daull, 0x3131a462f7a4f7c6ull, 0xd3d337bd5937598aull, 0xf2f28bff868b8674ull,
        0xd5d532b156325683ull, 0x8b8b430dc543c54eull, 0x6e6e59dceb59eb85ull, 0xdadab7afc2b7c218ull,
        0x01018c028f8c8f8eull, 0xb1b16479ac64ac1dull, 0x9c9cd2236dd26df1ull, 0x4949e0923be03b72ull,
        0xd8d8b4abc7b4c71full, 0xacacfa4315fa15b9ull, 0xf3f307fd090709faull, 0xcfcf25856f256fa0ull,
        0xcacaaf8feaafea20ull, 0xf4f48ef3898e897dull, 0x4747e98e20e92067ull, 0x1010182028182838ull,
        0x6f6fd5de64d5640bull, 0xf0f088fb83888373ull, 0x4a4a6f94b16fb1fbull, 0x5c5c72b8967296caull,
        0x383824706c246c54ull, 0x5757f1ae08f1085full, 0x7373c7e652c75221ull, 0x97975135f351f364ull,
        0xcbcb238d652365aeull, 0xa1a17c59847c8425ull, 0xe8e89ccbbf9cbf57ull, 0x3e3e217c6321635dull,
        0x9696dd377cdd7ceaull, 0x6161dcc27fdc7f1eull, 0x0d0d861a9186919cull, 0x0f0f851e9485949bull,
        0xe0e090dbab90ab4bull, 0x7c7c42f8c642c6baull, 0x7171c4e257c45726ull, 0xccccaa83e5aae529ull,
        0x9090d83b73d873e3ull, 0x0606050c0f050f09ull, 0xf7f701f5030103f4ull, 0x1c1c12383612362aull,
        0xc2c2a39ffea3fe3cull, 0x6a6a5fd4e15fe18bull, 0xaeaef94710f910beull, 0x6969d0d26bd06b02ull,
        0x1717912ea891a8bfull, 0x99995829e858e871ull, 0x3a3a277469276953ull, 0x2727b94ed0b9d0f7ull,
        0xd9d938a948384891ull, 0xebeb13cd351335deull, 0x2b2bb356ceb3cee5ull, 0x2222334455335577ull,
        0xd2d2bbbfd6bbd604ull, 0xa9a9704990709039ull, 0x0707890e80898087ull, 0x3333a766f2a7f2c1ull,
        0x2d2db65ac1b6c1ecull, 0x3c3c22786622665aull, 0x1515922aad92adb8ull, 0xc9c92089602060a9ull,
        0x87874915db49db5cull, 0xaaaaff4f1aff1ab0ull, 0x505078a0887888d8ull, 0xa5a57a518e7a8e2bull,
        0x03038f068a8f8a89ull, 0x5959f8b213f8134aull, 0x090980129b809b92ull, 0x1a1a173439173923ull,
        0x6565daca75da7510ull, 0xd7d731b553315384ull, 0x8484c61351c651d5ull, 0xd0d0b8bbd3b8d303ull,
        0x8282c31f5ec35edcull, 0x2929b052cbb0cbe2ull, 0x5a5a77b4997799c3ull, 0x1e1e113c3311332dull,
        0x7b7bcbf646cb463dull, 0xa8a8fc4b1ffc1fb7ull, 0x6d6dd6da61d6610cull, 0x2c2c3a584e3a4e62ull
      }
     }; // groestl_constants

    namespace groestl_functions
    {

        template <int R>
        static inline void
        roundP(uint64_t* x, uint64_t* y, uint64_t i)
        {
          for (int idx = 0; idx < R; idx++)
            x[idx] ^= (static_cast<uint64_t>(idx) << 4) ^ i;

          for (int c = R - 1; c >= 0; c--)
            y[c] = groestl_constants<void>::T[0][static_cast<unsigned char>(x[(c + 0) % R])]
                 ^ groestl_constants<void>::T[1][static_cast<unsigned char>(x[(c + 1) % R] >>  8)]
                 ^ groestl_constants<void>::T[2][static_cast<unsigned char>(x[(c + 2) % R] >> 16)]
                 ^ groestl_constants<void>::T[3][static_cast<unsigned char>(x[(c + 3) % R] >> 24)]
                 ^ groestl_constants<void>::T[4][static_cast<unsigned char>(x[(c + 4) % R] >> 32)]
                 ^ groestl_constants<void>::T[5][static_cast<unsigned char>(x[(c + 5) % R] >> 40)]
                 ^ groestl_constants<void>::T[6][static_cast<unsigned char>(x[(c + 6) % R] >> 48)]
                 ^ groestl_constants<void>::T[7][static_cast<unsigned char>(
                                      x[(c + (R == 16 ? 11 : 7)) % R] >> 56)
                                                ];
        }

        template <int R>
        static inline void
        roundQ(uint64_t* x, uint64_t* y, uint64_t i)
        {
          for (int idx = 0; idx < R; idx++)
            x[idx] ^= (0xffffffffffffffffull -
                      (static_cast<uint64_t>(idx) << 60)) ^ i;

          for (int c = R - 1; c >= 0; c--)
            y[c] = groestl_constants<void>::T[0][static_cast<unsigned char>(x[(c + 1) % R])]
                 ^ groestl_constants<void>::T[1][static_cast<unsigned char>(x[(c + 3) % R] >>  8)]
                 ^ groestl_constants<void>::T[2][static_cast<unsigned char>(x[(c + 5) % R] >> 16)]
                 ^ groestl_constants<void>::T[3][static_cast<unsigned char>(
                                      x[(c + (R == 16 ? 11 : 7)) % R] >> 24)
                                                ]
                 ^ groestl_constants<void>::T[4][static_cast<unsigned char>(x[(c + 0) % R] >> 32)]
                 ^ groestl_constants<void>::T[5][static_cast<unsigned char>(x[(c + 2) % R] >> 40)]
                 ^ groestl_constants<void>::T[6][static_cast<unsigned char>(x[(c + 4) % R] >> 48)]
                 ^ groestl_constants<void>::T[7][static_cast<unsigned char>(x[(c + 6) % R] >> 56)];
        }

        template <int R>
        static inline void
        transform(uint64_t* h, const uint64_t* m)
        {
          uint64_t AQ1[R], AQ2[R], AP1[R], AP2[R];

          for (int column = 0; column < R; column++)
          {
            AP1[column] = h[column] ^ m[column];
            AQ1[column] = m[column];
          }

          for (uint64_t r = 0; r < (R == 16 ? 14 : 10); r += 2)
          {
            roundP<R>(AP1, AP2,  r    );
            roundP<R>(AP2, AP1,  r + 1);
            roundQ<R>(AQ1, AQ2,  r         << 56);
            roundQ<R>(AQ2, AQ1, (r + 1ull) << 56);
          }

          for (int column = 0; column < R; column++)
          {
            h[column] = AP1[column] ^ AQ1[column] ^ h[column];
          }
        }

        template <int R>
        static inline void
        outputTransform(uint64_t* h)
        {
          uint64_t t1[R];
          uint64_t t2[R];

          for (int column = 0; column < R; column++) {
            t1[column] = h[column];
          }

          for (uint64_t r = 0; r < (R == 16 ? 14 : 10); r += 2) {
            roundP<R>(t1, t2, r);
            roundP<R>(t2, t1, r+1);
          }

          for (int column = 0; column < R; column++) {
            h[column] ^= t1[column];
          }
        }

    } // namespace groestl_functions

    class groestl_provider
    {
     public:
      static const bool is_xof = false;

      groestl_provider(size_t hashsize = 512)
      : hs{hashsize}
      {
        validate_hash_size(hashsize, 512);
      }

      ~groestl_provider()
      {
        clear();
      }

      inline void init()
      {
        pos = 0;
        total = 0;
        memset(&h[0], 0, sizeof(uint64_t)*16);
        uint64_t lohs = hash_size();
        h[lohs > 256 ? 15 : 7] = byteswap(lohs);
      }

      inline void update(const unsigned char* data, size_t len)
      {
        absorb_bytes(data, len,
          block_bytes(), block_bytes(),
          m.data(), pos, total,
          [this](const unsigned char* adata, size_t alen)
                { transform(adata, alen); });
      }

      inline void _final(unsigned char* hash)
      {
        m[pos++] = 0x80;
        total += pos * 8;
        size_t limit = block_bytes();
        if (pos > limit - 8)
        {
          if (limit != pos) memset(&m[pos], 0, limit - pos);
          transform(m.data(), 1);
          total += (block_bytes() - pos) * 8;
          pos = 0;
        }
        memset(&m[pos], 0, limit - 8 - pos);
        total += (block_bytes() - pos) * 8;
        uint64_t mlen = byteswap(total / 8 / block_bytes());
        memcpy(&m[limit - 8], &mlen, 64 / 8);
        transform(m.data(), 1);
        outputTransform();

        const unsigned char* s = reinterpret_cast<const unsigned char*>(h.data());
        for (size_t i = limit - hash_size() / 8, j = 0; i < limit; i++, j++)
          hash[j] = s[i];
      } // _final

      inline void clear()
      {
        zero_memory(h);
        zero_memory(m);
      }

      inline size_t hash_size() const { return hs; }

     private:

      inline size_t block_bytes() const { return hs > 256 ? 128 : 64; }

      inline void outputTransform()
      {
        if (hs > 256) groestl_functions::outputTransform<16>(&h[0]);
        else          groestl_functions::outputTransform< 8>(&h[0]);
      }

      inline void transform(const unsigned char* mp, size_t num_blks)
      {
        for (size_t blk = 0; blk < num_blks; blk++)
        {
          if (hs > 256)
            groestl_functions::transform<16>(&h[0],
              reinterpret_cast<const uint64_t*>(mp + block_bytes() * blk));
          else
            groestl_functions::transform< 8>(&h[0],
              reinterpret_cast<const uint64_t*>(mp + block_bytes() * blk));
        }
      }

      std::array<uint64_t, 16> h;
      std::array<unsigned char, 128> m;
      size_t hs;
      size_t pos;
      uint64_t total;

    }; // class groestl_provider

    template <typename T>
    struct jh_constants
    {
      static const uint64_t C[168];
    };

    template <typename T>
    const uint64_t jh_constants<T>::C[168] =
     {
      0x67f815dfa2ded572ull, 0x571523b70a15847bull, 0xf6875a4d90d6ab81ull, 0x402bd1c3c54f9f4eull,
      0x9cfa455ce03a98eaull, 0x9a99b26699d2c503ull, 0x8a53bbf2b4960266ull, 0x31a2db881a1456b5ull,
      0xdb0e199a5c5aa303ull, 0x1044c1870ab23f40ull, 0x1d959e848019051cull, 0xdccde75eadeb336full,
      0x416bbf029213ba10ull, 0xd027bbf7156578dcull, 0x5078aa3739812c0aull, 0xd3910041d2bf1a3full,
      0x907eccf60d5a2d42ull, 0xce97c0929c9f62ddull, 0xac442bc70ba75c18ull, 0x23fcc663d665dfd1ull,
      0x1ab8e09e036c6e97ull, 0xa8ec6c447e450521ull, 0xfa618e5dbb03f1eeull, 0x97818394b29796fdull,
      0x2f3003db37858e4aull, 0x956a9ffb2d8d672aull, 0x6c69b8f88173fe8aull, 0x14427fc04672c78aull,
      0xc45ec7bd8f15f4c5ull, 0x80bb118fa76f4475ull, 0xbc88e4aeb775de52ull, 0xf4a3a6981e00b882ull,
      0x1563a3a9338ff48eull, 0x89f9b7d524565faaull, 0xfde05a7c20edf1b6ull, 0x362c42065ae9ca36ull,
      0x3d98fe4e433529ceull, 0xa74b9a7374f93a53ull, 0x86814e6f591ff5d0ull, 0x9f5ad8af81ad9d0eull,
      0x6a6234ee670605a7ull, 0x2717b96ebe280b8bull, 0x3f1080c626077447ull, 0x7b487ec66f7ea0e0ull,
      0xc0a4f84aa50a550dull, 0x9ef18e979fe7e391ull, 0xd48d605081727686ull, 0x62b0e5f3415a9e7eull,
      0x7a205440ec1f9ffcull, 0x84c9f4ce001ae4e3ull, 0xd895fa9df594d74full, 0xa554c324117e2e55ull,
      0x286efebd2872df5bull, 0xb2c4a50fe27ff578ull, 0x2ed349eeef7c8905ull, 0x7f5928eb85937e44ull,
      0x4a3124b337695f70ull, 0x65e4d61df128865eull, 0xe720b95104771bc7ull, 0x8a87d423e843fe74ull,
      0xf2947692a3e8297dull, 0xc1d9309b097acbddull, 0xe01bdc5bfb301b1dull, 0xbf829cf24f4924daull,
      0xffbf70b431bae7a4ull, 0x48bcf8de0544320dull, 0x39d3bb5332fcae3bull, 0xa08b29e0c1c39f45ull,
      0x0f09aef7fd05c9e5ull, 0x34f1904212347094ull, 0x95ed44e301b771a2ull, 0x4a982f4f368e3be9ull,
      0x15f66ca0631d4088ull, 0xffaf52874b44c147ull, 0x30c60ae2f14abb7eull, 0xe68c6eccc5b67046ull,
      0x00ca4fbd56a4d5a4ull, 0xae183ec84b849ddaull, 0xadd1643045ce5773ull, 0x67255c1468cea6e8ull,
      0x16e10ecbf28cdaa3ull, 0x9a99949a5806e933ull, 0x7b846fc220b2601full, 0x1885d1a07facced1ull,
      0xd319dd8da15b5932ull, 0x46b4a5aac01c9a50ull, 0xba6b04e467633d9full, 0x7eee560bab19caf6ull,
      0x742128a9ea79b11full, 0xee51363b35f7bde9ull, 0x76d350755aac571dull, 0x01707da3fec2463aull,
      0x42d8a498afc135f7ull, 0x79676b9e20eced78ull, 0xa8db3aea15638341ull, 0x832c83324d3bc3faull,
      0xf347271c1f3b40a7ull, 0x9a762db734f04059ull, 0xfd4f21d26c4e3ee7ull, 0xef5957dc398dfdb8ull,
      0xdaeb492b490c9b8dull, 0x0d70f36849d7a25bull, 0x84558d7ad0ae3b7dull, 0x658ef8e4f0e9a5f5ull,
      0x533b1036f4a2b8a0ull, 0x5aec3e759e07a80cull, 0x4f88e85692946891ull, 0x4cbcbaf8555cb05bull,
      0x7b9487f3993bbbe3ull, 0x5d1c6b72d6f4da75ull, 0x6db334dc28acae64ull, 0x71db28b850a5346cull,
      0x2a518d10f2e261f8ull, 0xfc75dd593364dbe3ull, 0xa23fce43f1bcac1cull, 0xb043e8023cd1bb67ull,
      0x75a12988ca5b0a33ull, 0x5c5316b44d19347full, 0x1e4d790ec3943b92ull, 0x3fafeeb6d7757479ull,
      0x21391abef7d4a8eaull, 0x5127234c097ef45cull, 0xd23c32ba5324a326ull, 0xadd5a66d4a17a344ull,
      0x08c9f2afa63e1db5ull, 0x563c6b91983d5983ull, 0x4d608672a17cf84cull, 0xf6c76e08cc3ee246ull,
      0x5e76bcb1b333982full, 0x2ae6c4efa566d62bull, 0x36d4c1bee8b6f406ull, 0x6321efbc1582ee74ull,
      0x69c953f40d4ec1fdull, 0x26585806c45a7da7ull, 0x16fae0061614c17eull, 0x3f9d63283daf907eull,
      0x0cd29b00e3f2c9d2ull, 0x300cd4b730ceaa5full, 0x9832e0f216512a74ull, 0x9af8cee3d830eb0dull,
      0x9279f1b57b9ec54bull, 0xd36886046ee651ffull, 0x316796e6574d239bull, 0x05750a17f3a6e6ccull,
      0xce6c3213d98176b1ull, 0x62a205f88452173cull, 0x47154778b3cb2bf4ull, 0x486a9323825446ffull,
      0x65655e4e0758df38ull, 0x8e5086fc897cfcf2ull, 0x86ca0bd0442e7031ull, 0x4e477830a20940f0ull,
      0x8338f7d139eea065ull, 0xbd3a2ce437e95ef7ull, 0x6ff8130126b29721ull, 0xe7de9fefd1ed44a3ull,
      0xd992257615dfa08bull, 0xbe42dc12f6f7853cull, 0x7eb027ab7ceca7d8ull, 0xdea83eaada7d8d53ull,
      0xd86902bd93ce25aaull, 0xf908731afd43f65aull, 0xa5194a17daef5fc0ull, 0x6a21fd4c33664d97ull,
      0x701541db3198b435ull, 0x9b54cdedbb0f1eeaull, 0x72409751a163d09aull, 0xe26f4791bf9d75f6ull
     }; // jh_constants

    namespace jh_functions
    {
        static inline void bitswap(uint64_t& x,
                                   uint64_t mask,
                                   int shift)
        {
          x = ((x & mask) << shift) | ((x & ~mask) >> shift);
        }

        static inline void bitswap(std::array<uint64_t, 16>& H,
                                   uint64_t mask,
                                   int shift)
        {
          bitswap(H[ 2], mask, shift);
          bitswap(H[ 3], mask, shift);
          bitswap(H[ 6], mask, shift);
          bitswap(H[ 7], mask, shift);
          bitswap(H[10], mask, shift);
          bitswap(H[11], mask, shift);
          bitswap(H[14], mask, shift);
          bitswap(H[15], mask, shift);
        }

        static inline void Sbitsli(uint64_t& x0a,
                                   uint64_t& x0b,
                                   uint64_t& x1a,
                                   uint64_t& x1b,
                                   uint64_t& x2a,
                                   uint64_t& x2b,
                                   uint64_t& x3a,
                                   uint64_t& x3b,
                                   uint64_t ca,
                                   uint64_t cb)
        {
          x3a  = ~x3a;
          x3b  = ~x3b;
          x0a ^= ( ca & ~x2a);
          x0b ^= ( cb & ~x2b);
          uint64_t ta = ca ^ (x0a & x1a);
          uint64_t tb = cb ^ (x0b & x1b);
          x0a ^= ( x2a &  x3a);
          x0b ^= ( x2b &  x3b);
          x3a ^= (~x1a &  x2a);
          x3b ^= (~x1b &  x2b);
          x1a ^= ( x0a &  x2a);
          x1b ^= ( x0b &  x2b);
          x2a ^= ( x0a & ~x3a);
          x2b ^= ( x0b & ~x3b);
          x0a ^= ( x1a |  x3a);
          x0b ^= ( x1b |  x3b);
          x3a ^= ( x1a &  x2a);
          x3b ^= ( x1b &  x2b);
          x1a ^= ( ta  &  x0a);
          x1b ^= ( tb  &  x0b);
          x2a ^=   ta;
          x2b ^=   tb;
        }

        static inline void Lbitsli(std::array<uint64_t, 16>& H)
        {
          H[ 2] ^= H[ 4];
          H[ 3] ^= H[ 5];
          H[ 6] ^= H[ 8];
          H[ 7] ^= H[ 9];
          H[10] ^= H[12] ^ H[0];
          H[11] ^= H[13] ^ H[1];
          H[14] ^= H[ 0];
          H[15] ^= H[ 1];
          H[ 0] ^= H[ 6];
          H[ 1] ^= H[ 7];
          H[ 4] ^= H[10];
          H[ 5] ^= H[11];
          H[ 8] ^= H[14] ^ H[2];
          H[ 9] ^= H[15] ^ H[3];
          H[12] ^= H[ 2];
          H[13] ^= H[ 3];
        }

        static inline void round(std::array<uint64_t, 16>& H, int r, uint64_t mask, int shift)
        {
          Sbitsli(H[0],  H[ 1], H[ 4],
                  H[5],  H[ 8], H[ 9],
                  H[12], H[13], jh_constants<void>::C[r * 4 + 0],
                                jh_constants<void>::C[r * 4 + 1]);
          Sbitsli(H[2],  H[ 3], H[ 6],
                  H[7],  H[10], H[11],
                  H[14], H[15], jh_constants<void>::C[r * 4 + 2],
                                jh_constants<void>::C[r * 4 + 3]);
          Lbitsli(H);
          if (shift) bitswap(H, mask, shift);
        }

    } // namespace jh_functions

    class jh_provider
    {
     public:
      static const bool is_xof = false;

      jh_provider(size_t hashsize = 512)
      : hs{hashsize}
      {
        validate_hash_size(hashsize, 512);
      }

      ~jh_provider()
      {
        clear();
      }

      inline void init()
      {
        pos = 0;
        total = 0;
        zero_memory(H);
        H[0] = byteswap(static_cast<uint16_t>(hs));
        unsigned char msg[64];
        memset(msg, 0, sizeof(msg));
        transform(msg, 1);
      }

      inline void update(const unsigned char* data, size_t len)
      {
        absorb_bytes(data, len,
          64, 64,
          m.data(), pos, total,
          [this](const unsigned char* adata, size_t alen)
                { transform(adata, alen); });
      }

      inline void _final(unsigned char* hash)
      {
        total += pos * 8;
        m[pos++] = 0x80;
        size_t limit = 64;
        if (pos > 1)
        {
          if (limit != pos) memset(&m[pos], 0, limit - pos);
          transform(&m[0], 1);
          pos = 0;
        }
        memset(&m[pos], 0, limit - 8 - pos);
        total += 0;
        uint64_t mlen = byteswap(total);
        memcpy(&m[limit - 8], &mlen, 64 / 8);
        transform(&m[0], 1);

        memcpy(hash,
               reinterpret_cast<unsigned char*>(H.data()) +
               128 - hash_size() / 8,
               hash_size() / 8);
      }

      inline void clear()
      {
        zero_memory(H);
        zero_memory(m);
      }

      inline size_t hash_size() const { return hs; }

     private:
      inline void transform(const unsigned char* mp, size_t num_blks)
      {
        for (uint64_t blk = 0; blk < num_blks; blk++)
        {
          const uint64_t* M = (const uint64_t*)(((const unsigned char*)mp) + blk * 64);
          H[0] ^= M[0];
          H[1] ^= M[1];
          H[2] ^= M[2];
          H[3] ^= M[3];
          H[4] ^= M[4];
          H[5] ^= M[5];
          H[6] ^= M[6];
          H[7] ^= M[7];

          // partially unroll
          for (int r = 0; r < 42; r += 7)
          {
            jh_functions::round(H, r,     0x5555555555555555ULL,  1);
            jh_functions::round(H, r + 1, 0x3333333333333333ULL,  2);
            jh_functions::round(H, r + 2, 0x0f0f0f0f0f0f0f0fULL,  4);
            jh_functions::round(H, r + 3, 0x00ff00ff00ff00ffULL,  8);
            jh_functions::round(H, r + 4, 0x0000ffff0000ffffULL, 16);
            jh_functions::round(H, r + 5, 0x00000000ffffffffULL, 32);
            jh_functions::round(H, r + 6, 0, 0);

            std::swap(H[ 2], H[ 3]);
            std::swap(H[ 6], H[ 7]);
            std::swap(H[10], H[11]);
            std::swap(H[14], H[15]);
          }

          H[ 8] ^= M[0];
          H[ 9] ^= M[1];
          H[10] ^= M[2];
          H[11] ^= M[3];
          H[12] ^= M[4];
          H[13] ^= M[5];
          H[14] ^= M[6];
          H[15] ^= M[7];
        }
      }

      std::array<uint64_t, 16> H;
      std::array<unsigned char, 64> m;
      size_t hs;
      size_t pos;
      uint64_t total;

    }; // class jh_provider

    template <typename V>
    struct kupyna_constants
    {
      static const uint64_t T[8][256];
    };

    template <typename V>
    const uint64_t kupyna_constants<V>::T[8][256] =
     {
      {
        0xa832a829d77f9aa8ull, 0x4352432297d41143ull, 0x5f3e5fc2df80615full, 0x061e063014121806ull,
        0x6bda6b7f670cb16bull, 0x75bc758f2356c975ull, 0x6cc16c477519ad6cull, 0x592059f2cb927959ull,
        0x71a871af3b4ad971ull, 0xdf84dfb6f8275bdfull, 0x87a1874c35b22687ull, 0x95fb95dc59cc6e95ull,
        0x174b17b872655c17ull, 0xf017f0d31aeae7f0ull, 0xd89fd88eea3247d8ull, 0x092d0948363f2409ull,
        0x6dc46d4f731ea96dull, 0xf318f3cb10e3ebf3ull, 0x1d691de84e53741dull, 0xcbc0cb16804b0bcbull,
        0xc9cac9068c4503c9ull, 0x4d644d52b3fe294dull, 0x2c9c2c7de8c4b02cull, 0xaf29af11c56a86afull,
        0x798079ef0b72f979ull, 0xe047e0537a9aa7e0ull, 0x97f197cc55c26697ull, 0xfd2efdbb34c9d3fdull,
        0x6fce6f5f7f10a16full, 0x4b7a4b62a7ec314bull, 0x454c451283c60945ull, 0x39dd39d596afe439ull,
        0x3ec63eed84baf83eull, 0xdd8edda6f42953ddull, 0xa315a371ed4eb6a3ull, 0x4f6e4f42bff0214full,
        0xb45eb4c99f2beab4ull, 0xb654b6d99325e2b6ull, 0x9ac89aa47be1529aull, 0x0e360e70242a380eull,
        0x1f631ff8425d7c1full, 0xbf79bf91a51ac6bfull, 0x154115a87e6b5415ull, 0xe142e15b7c9da3e1ull,
        0x49704972abe23949ull, 0xd2bdd2ded6046fd2ull, 0x93e593ec4dde7693ull, 0xc6f9c67eae683fc6ull,
        0x92e092e44bd97292ull, 0x72a772b73143d572ull, 0x9edc9e8463fd429eull, 0x61f8612f5b3a9961ull,
        0xd1b2d1c6dc0d63d1ull, 0x63f2633f57349163ull, 0xfa35fa8326dccffaull, 0xee71ee235eb09feeull,
        0xf403f4f302f6f7f4ull, 0x197d19c8564f6419ull, 0xd5a6d5e6c41173d5ull, 0xad23ad01c9648eadull,
        0x582558facd957d58ull, 0xa40ea449ff5baaa4ull, 0xbb6dbbb1bd06d6bbull, 0xa11fa161e140bea1ull,
        0xdc8bdcaef22e57dcull, 0xf21df2c316e4eff2ull, 0x83b5836c2dae3683ull, 0x37eb37a5b285dc37ull,
        0x4257422a91d31542ull, 0xe453e4736286b7e4ull, 0x7a8f7af7017bf57aull, 0x32fa328dac9ec832ull,
        0x9cd69c946ff34a9cull, 0xccdbcc2e925e17ccull, 0xab3dab31dd7696abull, 0x4a7f4a6aa1eb354aull,
        0x8f898f0c058a068full, 0x6ecb6e577917a56eull, 0x04140420181c1004ull, 0x27bb2725d2f59c27ull,
        0x2e962e6de4cab82eull, 0xe75ce76b688fbbe7ull, 0xe24de2437694afe2ull, 0x5a2f5aeac19b755aull,
        0x96f496c453c56296ull, 0x164e16b074625816ull, 0x23af2305cae98c23ull, 0x2b872b45fad1ac2bull,
        0xc2edc25eb6742fc2ull, 0x65ec650f43268965ull, 0x66e36617492f8566ull, 0x0f330f78222d3c0full,
        0xbc76bc89af13cabcull, 0xa937a921d1789ea9ull, 0x474647028fc80147ull, 0x415841329bda1941ull,
        0x34e434bdb88cd034ull, 0x4875487aade53d48ull, 0xfc2bfcb332ced7fcull, 0xb751b7d19522e6b7ull,
        0x6adf6a77610bb56aull, 0x88928834179f1a88ull, 0xa50ba541f95caea5ull, 0x530253a2f7a45153ull,
        0x86a4864433b52286ull, 0xf93af99b2cd5c3f9ull, 0x5b2a5be2c79c715bull, 0xdb90db96e03b4bdbull,
        0x38d838dd90a8e038ull, 0x7b8a7bff077cf17bull, 0xc3e8c356b0732bc3ull, 0x1e661ef0445a781eull,
        0x22aa220dccee8822ull, 0x33ff3385aa99cc33ull, 0x24b4243dd8fc9024ull, 0x2888285df0d8a028ull,
        0x36ee36adb482d836ull, 0xc7fcc776a86f3bc7ull, 0xb240b2f98b39f2b2ull, 0x3bd73bc59aa1ec3bull,
        0x8e8c8e04038d028eull, 0x77b6779f2f58c177ull, 0xba68bab9bb01d2baull, 0xf506f5fb04f1f3f5ull,
        0x144414a0786c5014ull, 0x9fd99f8c65fa469full, 0x0828084030382008ull, 0x551c5592e3b64955ull,
        0x9bcd9bac7de6569bull, 0x4c614c5ab5f92d4cull, 0xfe21fea33ec0dffeull, 0x60fd60275d3d9d60ull,
        0x5c315cdad5896d5cull, 0xda95da9ee63c4fdaull, 0x187818c050486018ull, 0x4643460a89cf0546ull,
        0xcddecd26945913cdull, 0x7d947dcf136ee97dull, 0x21a52115c6e78421ull, 0xb04ab0e98737fab0ull,
        0x3fc33fe582bdfc3full, 0x1b771bd85a416c1bull, 0x8997893c11981e89ull, 0xff24ffab38c7dbffull,
        0xeb60eb0b40ab8bebull, 0x84ae84543fbb2a84ull, 0x69d0696f6b02b969ull, 0x3ad23acd9ca6e83aull,
        0x9dd39d9c69f44e9dull, 0xd7acd7f6c81f7bd7ull, 0xd3b8d3d6d0036bd3ull, 0x70ad70a73d4ddd70ull,
        0x67e6671f4f288167ull, 0x405d403a9ddd1d40ull, 0xb55bb5c1992ceeb5ull, 0xde81debefe205fdeull,
        0x5d345dd2d38e695dull, 0x30f0309da090c030ull, 0x91ef91fc41d07e91ull, 0xb14fb1e18130feb1ull,
        0x788578e70d75fd78ull, 0x1155118866774411ull, 0x0105010806070401ull, 0xe556e57b6481b3e5ull,
        0x0000000000000000ull, 0x68d568676d05bd68ull, 0x98c298b477ef5a98ull, 0xa01aa069e747baa0ull,
        0xc5f6c566a46133c5ull, 0x020a02100c0e0802ull, 0xa604a659f355a2a6ull, 0x74b974872551cd74ull,
        0x2d992d75eec3b42dull, 0x0b270b583a312c0bull, 0xa210a279eb49b2a2ull, 0x76b37697295fc576ull,
        0xb345b3f18d3ef6b3ull, 0xbe7cbe99a31dc2beull, 0xced1ce3e9e501fceull, 0xbd73bd81a914cebdull,
        0xae2cae19c36d82aeull, 0xe96ae91b4ca583e9ull, 0x8a988a241b91128aull, 0x31f53195a697c431ull,
        0x1c6c1ce04854701cull, 0xec7bec3352be97ecull, 0xf112f1db1cede3f1ull, 0x99c799bc71e85e99ull,
        0x94fe94d45fcb6a94ull, 0xaa38aa39db7192aaull, 0xf609f6e30ef8fff6ull, 0x26be262dd4f29826ull,
        0x2f932f65e2cdbc2full, 0xef74ef2b58b79befull, 0xe86fe8134aa287e8ull, 0x8c868c140f830a8cull,
        0x35e135b5be8bd435ull, 0x030f03180a090c03ull, 0xd4a3d4eec21677d4ull, 0x7f9e7fdf1f60e17full,
        0xfb30fb8b20dbcbfbull, 0x051105281e1b1405ull, 0xc1e2c146bc7d23c1ull, 0x5e3b5ecad987655eull,
        0x90ea90f447d77a90ull, 0x20a0201dc0e08020ull, 0x3dc93df58eb3f43dull, 0x82b082642ba93282ull,
        0xf70cf7eb08fffbf7ull, 0xea65ea0346ac8feaull, 0x0a220a503c36280aull, 0x0d390d682e23340dull,
        0x7e9b7ed71967e57eull, 0xf83ff8932ad2c7f8ull, 0x500d50bafdad5d50ull, 0x1a721ad05c46681aull,
        0xc4f3c46ea26637c4ull, 0x071b073812151c07ull, 0x57165782efb84157ull, 0xb862b8a9b70fdab8ull,
        0x3ccc3cfd88b4f03cull, 0x62f7623751339562ull, 0xe348e34b7093abe3ull, 0xc8cfc80e8a4207c8ull,
        0xac26ac09cf638aacull, 0x520752aaf1a35552ull, 0x64e9640745218d64ull, 0x1050108060704010ull,
        0xd0b7d0ceda0a67d0ull, 0xd99ad986ec3543d9ull, 0x135f13986a794c13ull, 0x0c3c0c602824300cull,
        0x125a12906c7e4812ull, 0x298d2955f6dfa429ull, 0x510851b2fbaa5951ull, 0xb967b9a1b108deb9ull,
        0xcfd4cf3698571bcfull, 0xd6a9d6fece187fd6ull, 0x73a273bf3744d173ull, 0x8d838d1c09840e8dull,
        0x81bf817c21a03e81ull, 0x5419549ae5b14d54ull, 0xc0e7c04eba7a27c0ull, 0xed7eed3b54b993edull,
        0x4e6b4e4ab9f7254eull, 0x4449441a85c10d44ull, 0xa701a751f552a6a7ull, 0x2a822a4dfcd6a82aull,
        0x85ab855c39bc2e85ull, 0x25b12535defb9425ull, 0xe659e6636e88bfe6ull, 0xcac5ca1e864c0fcaull,
        0x7c917cc71569ed7cull, 0x8b9d8b2c1d96168bull, 0x5613568ae9bf4556ull, 0x80ba807427a73a80ull
      },
      {
        0xd1ce3e9e501fceceull, 0x6dbbb1bd06d6bbbbull, 0x60eb0b40ab8bebebull, 0xe092e44bd9729292ull,
        0x65ea0346ac8feaeaull, 0xc0cb16804b0bcbcbull, 0x5f13986a794c1313ull, 0xe2c146bc7d23c1c1ull,
        0x6ae91b4ca583e9e9ull, 0xd23acd9ca6e83a3aull, 0xa9d6fece187fd6d6ull, 0x40b2f98b39f2b2b2ull,
        0xbdd2ded6046fd2d2ull, 0xea90f447d77a9090ull, 0x4b17b872655c1717ull, 0x3ff8932ad2c7f8f8ull,
        0x57422a91d3154242ull, 0x4115a87e6b541515ull, 0x13568ae9bf455656ull, 0x5eb4c99f2beab4b4ull,
        0xec650f4326896565ull, 0x6c1ce04854701c1cull, 0x928834179f1a8888ull, 0x52432297d4114343ull,
        0xf6c566a46133c5c5ull, 0x315cdad5896d5c5cull, 0xee36adb482d83636ull, 0x68bab9bb01d2babaull,
        0x06f5fb04f1f3f5f5ull, 0x165782efb8415757ull, 0xe6671f4f28816767ull, 0x838d1c09840e8d8dull,
        0xf53195a697c43131ull, 0x09f6e30ef8fff6f6ull, 0xe9640745218d6464ull, 0x2558facd957d5858ull,
        0xdc9e8463fd429e9eull, 0x03f4f302f6f7f4f4ull, 0xaa220dccee882222ull, 0x38aa39db7192aaaaull,
        0xbc758f2356c97575ull, 0x330f78222d3c0f0full, 0x0a02100c0e080202ull, 0x4fb1e18130feb1b1ull,
        0x84dfb6f8275bdfdfull, 0xc46d4f731ea96d6dull, 0xa273bf3744d17373ull, 0x644d52b3fe294d4dull,
        0x917cc71569ed7c7cull, 0xbe262dd4f2982626ull, 0x962e6de4cab82e2eull, 0x0cf7eb08fffbf7f7ull,
        0x2808403038200808ull, 0x345dd2d38e695d5dull, 0x49441a85c10d4444ull, 0xc63eed84baf83e3eull,
        0xd99f8c65fa469f9full, 0x4414a0786c501414ull, 0xcfc80e8a4207c8c8ull, 0x2cae19c36d82aeaeull,
        0x19549ae5b14d5454ull, 0x5010806070401010ull, 0x9fd88eea3247d8d8ull, 0x76bc89af13cabcbcull,
        0x721ad05c46681a1aull, 0xda6b7f670cb16b6bull, 0xd0696f6b02b96969ull, 0x18f3cb10e3ebf3f3ull,
        0x73bd81a914cebdbdull, 0xff3385aa99cc3333ull, 0x3dab31dd7696ababull, 0x35fa8326dccffafaull,
        0xb2d1c6dc0d63d1d1ull, 0xcd9bac7de6569b9bull, 0xd568676d05bd6868ull, 0x6b4e4ab9f7254e4eull,
        0x4e16b07462581616ull, 0xfb95dc59cc6e9595ull, 0xef91fc41d07e9191ull, 0x71ee235eb09feeeeull,
        0x614c5ab5f92d4c4cull, 0xf2633f5734916363ull, 0x8c8e04038d028e8eull, 0x2a5be2c79c715b5bull,
        0xdbcc2e925e17ccccull, 0xcc3cfd88b4f03c3cull, 0x7d19c8564f641919ull, 0x1fa161e140bea1a1ull,
        0xbf817c21a03e8181ull, 0x704972abe2394949ull, 0x8a7bff077cf17b7bull, 0x9ad986ec3543d9d9ull,
        0xce6f5f7f10a16f6full, 0xeb37a5b285dc3737ull, 0xfd60275d3d9d6060ull, 0xc5ca1e864c0fcacaull,
        0x5ce76b688fbbe7e7ull, 0x872b45fad1ac2b2bull, 0x75487aade53d4848ull, 0x2efdbb34c9d3fdfdull,
        0xf496c453c5629696ull, 0x4c451283c6094545ull, 0x2bfcb332ced7fcfcull, 0x5841329bda194141ull,
        0x5a12906c7e481212ull, 0x390d682e23340d0dull, 0x8079ef0b72f97979ull, 0x56e57b6481b3e5e5ull,
        0x97893c11981e8989ull, 0x868c140f830a8c8cull, 0x48e34b7093abe3e3ull, 0xa0201dc0e0802020ull,
        0xf0309da090c03030ull, 0x8bdcaef22e57dcdcull, 0x51b7d19522e6b7b7ull, 0xc16c477519ad6c6cull,
        0x7f4a6aa1eb354a4aull, 0x5bb5c1992ceeb5b5ull, 0xc33fe582bdfc3f3full, 0xf197cc55c2669797ull,
        0xa3d4eec21677d4d4ull, 0xf762375133956262ull, 0x992d75eec3b42d2dull, 0x1e06301412180606ull,
        0x0ea449ff5baaa4a4ull, 0x0ba541f95caea5a5ull, 0xb5836c2dae368383ull, 0x3e5fc2df80615f5full,
        0x822a4dfcd6a82a2aull, 0x95da9ee63c4fdadaull, 0xcac9068c4503c9c9ull, 0x0000000000000000ull,
        0x9b7ed71967e57e7eull, 0x10a279eb49b2a2a2ull, 0x1c5592e3b6495555ull, 0x79bf91a51ac6bfbfull,
        0x5511886677441111ull, 0xa6d5e6c41173d5d5ull, 0xd69c946ff34a9c9cull, 0xd4cf3698571bcfcfull,
        0x360e70242a380e0eull, 0x220a503c36280a0aull, 0xc93df58eb3f43d3dull, 0x0851b2fbaa595151ull,
        0x947dcf136ee97d7dull, 0xe593ec4dde769393ull, 0x771bd85a416c1b1bull, 0x21fea33ec0dffefeull,
        0xf3c46ea26637c4c4ull, 0x4647028fc8014747ull, 0x2d0948363f240909ull, 0xa4864433b5228686ull,
        0x270b583a312c0b0bull, 0x898f0c058a068f8full, 0xd39d9c69f44e9d9dull, 0xdf6a77610bb56a6aull,
        0x1b073812151c0707ull, 0x67b9a1b108deb9b9ull, 0x4ab0e98737fab0b0ull, 0xc298b477ef5a9898ull,
        0x7818c05048601818ull, 0xfa328dac9ec83232ull, 0xa871af3b4ad97171ull, 0x7a4b62a7ec314b4bull,
        0x74ef2b58b79befefull, 0xd73bc59aa1ec3b3bull, 0xad70a73d4ddd7070ull, 0x1aa069e747baa0a0ull,
        0x53e4736286b7e4e4ull, 0x5d403a9ddd1d4040ull, 0x24ffab38c7dbffffull, 0xe8c356b0732bc3c3ull,
        0x37a921d1789ea9a9ull, 0x59e6636e88bfe6e6ull, 0x8578e70d75fd7878ull, 0x3af99b2cd5c3f9f9ull,
        0x9d8b2c1d96168b8bull, 0x43460a89cf054646ull, 0xba807427a73a8080ull, 0x661ef0445a781e1eull,
        0xd838dd90a8e03838ull, 0x42e15b7c9da3e1e1ull, 0x62b8a9b70fdab8b8ull, 0x32a829d77f9aa8a8ull,
        0x47e0537a9aa7e0e0ull, 0x3c0c602824300c0cull, 0xaf2305cae98c2323ull, 0xb37697295fc57676ull,
        0x691de84e53741d1dull, 0xb12535defb942525ull, 0xb4243dd8fc902424ull, 0x1105281e1b140505ull,
        0x12f1db1cede3f1f1ull, 0xcb6e577917a56e6eull, 0xfe94d45fcb6a9494ull, 0x88285df0d8a02828ull,
        0xc89aa47be1529a9aull, 0xae84543fbb2a8484ull, 0x6fe8134aa287e8e8ull, 0x15a371ed4eb6a3a3ull,
        0x6e4f42bff0214f4full, 0xb6779f2f58c17777ull, 0xb8d3d6d0036bd3d3ull, 0xab855c39bc2e8585ull,
        0x4de2437694afe2e2ull, 0x0752aaf1a3555252ull, 0x1df2c316e4eff2f2ull, 0xb082642ba9328282ull,
        0x0d50bafdad5d5050ull, 0x8f7af7017bf57a7aull, 0x932f65e2cdbc2f2full, 0xb974872551cd7474ull,
        0x0253a2f7a4515353ull, 0x45b3f18d3ef6b3b3ull, 0xf8612f5b3a996161ull, 0x29af11c56a86afafull,
        0xdd39d596afe43939ull, 0xe135b5be8bd43535ull, 0x81debefe205fdedeull, 0xdecd26945913cdcdull,
        0x631ff8425d7c1f1full, 0xc799bc71e85e9999ull, 0x26ac09cf638aacacull, 0x23ad01c9648eadadull,
        0xa772b73143d57272ull, 0x9c2c7de8c4b02c2cull, 0x8edda6f42953ddddull, 0xb7d0ceda0a67d0d0ull,
        0xa1874c35b2268787ull, 0x7cbe99a31dc2bebeull, 0x3b5ecad987655e5eull, 0x04a659f355a2a6a6ull,
        0x7bec3352be97ececull, 0x140420181c100404ull, 0xf9c67eae683fc6c6ull, 0x0f03180a090c0303ull,
        0xe434bdb88cd03434ull, 0x30fb8b20dbcbfbfbull, 0x90db96e03b4bdbdbull, 0x2059f2cb92795959ull,
        0x54b6d99325e2b6b6ull, 0xedc25eb6742fc2c2ull, 0x0501080607040101ull, 0x17f0d31aeae7f0f0ull,
        0x2f5aeac19b755a5aull, 0x7eed3b54b993ededull, 0x01a751f552a6a7a7ull, 0xe36617492f856666ull,
        0xa52115c6e7842121ull, 0x9e7fdf1f60e17f7full, 0x988a241b91128a8aull, 0xbb2725d2f59c2727ull,
        0xfcc776a86f3bc7c7ull, 0xe7c04eba7a27c0c0ull, 0x8d2955f6dfa42929ull, 0xacd7f6c81f7bd7d7ull
      },
      {
        0x93ec4dde769393e5ull, 0xd986ec3543d9d99aull, 0x9aa47be1529a9ac8ull, 0xb5c1992ceeb5b55bull,
        0x98b477ef5a9898c2ull, 0x220dccee882222aaull, 0x451283c60945454cull, 0xfcb332ced7fcfc2bull,
        0xbab9bb01d2baba68ull, 0x6a77610bb56a6adfull, 0xdfb6f8275bdfdf84ull, 0x02100c0e0802020aull,
        0x9f8c65fa469f9fd9ull, 0xdcaef22e57dcdc8bull, 0x51b2fbaa59515108ull, 0x59f2cb9279595920ull,
        0x4a6aa1eb354a4a7full, 0x17b872655c17174bull, 0x2b45fad1ac2b2b87ull, 0xc25eb6742fc2c2edull,
        0x94d45fcb6a9494feull, 0xf4f302f6f7f4f403ull, 0xbbb1bd06d6bbbb6dull, 0xa371ed4eb6a3a315ull,
        0x62375133956262f7ull, 0xe4736286b7e4e453ull, 0x71af3b4ad97171a8ull, 0xd4eec21677d4d4a3ull,
        0xcd26945913cdcddeull, 0x70a73d4ddd7070adull, 0x16b074625816164eull, 0xe15b7c9da3e1e142ull,
        0x4972abe239494970ull, 0x3cfd88b4f03c3cccull, 0xc04eba7a27c0c0e7ull, 0xd88eea3247d8d89full,
        0x5cdad5896d5c5c31ull, 0x9bac7de6569b9bcdull, 0xad01c9648eadad23ull, 0x855c39bc2e8585abull,
        0x53a2f7a451535302ull, 0xa161e140bea1a11full, 0x7af7017bf57a7a8full, 0xc80e8a4207c8c8cfull,
        0x2d75eec3b42d2d99ull, 0xe0537a9aa7e0e047ull, 0xd1c6dc0d63d1d1b2ull, 0x72b73143d57272a7ull,
        0xa659f355a2a6a604ull, 0x2c7de8c4b02c2c9cull, 0xc46ea26637c4c4f3ull, 0xe34b7093abe3e348ull,
        0x7697295fc57676b3ull, 0x78e70d75fd787885ull, 0xb7d19522e6b7b751ull, 0xb4c99f2beab4b45eull,
        0x0948363f2409092dull, 0x3bc59aa1ec3b3bd7ull, 0x0e70242a380e0e36ull, 0x41329bda19414158ull,
        0x4c5ab5f92d4c4c61ull, 0xdebefe205fdede81ull, 0xb2f98b39f2b2b240ull, 0x90f447d77a9090eaull,
        0x2535defb942525b1ull, 0xa541f95caea5a50bull, 0xd7f6c81f7bd7d7acull, 0x03180a090c03030full,
        0x1188667744111155ull, 0x0000000000000000ull, 0xc356b0732bc3c3e8ull, 0x2e6de4cab82e2e96ull,
        0x92e44bd9729292e0ull, 0xef2b58b79befef74ull, 0x4e4ab9f7254e4e6bull, 0x12906c7e4812125aull,
        0x9d9c69f44e9d9dd3ull, 0x7dcf136ee97d7d94ull, 0xcb16804b0bcbcbc0ull, 0x35b5be8bd43535e1ull,
        0x1080607040101050ull, 0xd5e6c41173d5d5a6ull, 0x4f42bff0214f4f6eull, 0x9e8463fd429e9edcull,
        0x4d52b3fe294d4d64ull, 0xa921d1789ea9a937ull, 0x5592e3b64955551cull, 0xc67eae683fc6c6f9ull,
        0xd0ceda0a67d0d0b7ull, 0x7bff077cf17b7b8aull, 0x18c0504860181878ull, 0x97cc55c2669797f1ull,
        0xd3d6d0036bd3d3b8ull, 0x36adb482d83636eeull, 0xe6636e88bfe6e659ull, 0x487aade53d484875ull,
        0x568ae9bf45565613ull, 0x817c21a03e8181bfull, 0x8f0c058a068f8f89ull, 0x779f2f58c17777b6ull,
        0xcc2e925e17ccccdbull, 0x9c946ff34a9c9cd6ull, 0xb9a1b108deb9b967ull, 0xe2437694afe2e24dull,
        0xac09cf638aacac26ull, 0xb8a9b70fdab8b862ull, 0x2f65e2cdbc2f2f93ull, 0x15a87e6b54151541ull,
        0xa449ff5baaa4a40eull, 0x7cc71569ed7c7c91ull, 0xda9ee63c4fdada95ull, 0x38dd90a8e03838d8ull,
        0x1ef0445a781e1e66ull, 0x0b583a312c0b0b27ull, 0x05281e1b14050511ull, 0xd6fece187fd6d6a9ull,
        0x14a0786c50141444ull, 0x6e577917a56e6ecbull, 0x6c477519ad6c6cc1ull, 0x7ed71967e57e7e9bull,
        0x6617492f856666e3ull, 0xfdbb34c9d3fdfd2eull, 0xb1e18130feb1b14full, 0xe57b6481b3e5e556ull,
        0x60275d3d9d6060fdull, 0xaf11c56a86afaf29ull, 0x5ecad987655e5e3bull, 0x3385aa99cc3333ffull,
        0x874c35b2268787a1ull, 0xc9068c4503c9c9caull, 0xf0d31aeae7f0f017ull, 0x5dd2d38e695d5d34ull,
        0x6d4f731ea96d6dc4ull, 0x3fe582bdfc3f3fc3ull, 0x8834179f1a888892ull, 0x8d1c09840e8d8d83ull,
        0xc776a86f3bc7c7fcull, 0xf7eb08fffbf7f70cull, 0x1de84e53741d1d69ull, 0xe91b4ca583e9e96aull,
        0xec3352be97ecec7bull, 0xed3b54b993eded7eull, 0x807427a73a8080baull, 0x2955f6dfa429298dull,
        0x2725d2f59c2727bbull, 0xcf3698571bcfcfd4ull, 0x99bc71e85e9999c7ull, 0xa829d77f9aa8a832ull,
        0x50bafdad5d50500dull, 0x0f78222d3c0f0f33ull, 0x37a5b285dc3737ebull, 0x243dd8fc902424b4ull,
        0x285df0d8a0282888ull, 0x309da090c03030f0ull, 0x95dc59cc6e9595fbull, 0xd2ded6046fd2d2bdull,
        0x3eed84baf83e3ec6ull, 0x5be2c79c715b5b2aull, 0x403a9ddd1d40405dull, 0x836c2dae368383b5ull,
        0xb3f18d3ef6b3b345ull, 0x696f6b02b96969d0ull, 0x5782efb841575716ull, 0x1ff8425d7c1f1f63ull,
        0x073812151c07071bull, 0x1ce04854701c1c6cull, 0x8a241b91128a8a98ull, 0xbc89af13cabcbc76ull,
        0x201dc0e0802020a0ull, 0xeb0b40ab8bebeb60ull, 0xce3e9e501fceced1ull, 0x8e04038d028e8e8cull,
        0xab31dd7696abab3dull, 0xee235eb09feeee71ull, 0x3195a697c43131f5ull, 0xa279eb49b2a2a210ull,
        0x73bf3744d17373a2ull, 0xf99b2cd5c3f9f93aull, 0xca1e864c0fcacac5ull, 0x3acd9ca6e83a3ad2ull,
        0x1ad05c46681a1a72ull, 0xfb8b20dbcbfbfb30ull, 0x0d682e23340d0d39ull, 0xc146bc7d23c1c1e2ull,
        0xfea33ec0dffefe21ull, 0xfa8326dccffafa35ull, 0xf2c316e4eff2f21dull, 0x6f5f7f10a16f6fceull,
        0xbd81a914cebdbd73ull, 0x96c453c5629696f4ull, 0xdda6f42953dddd8eull, 0x432297d411434352ull,
        0x52aaf1a355525207ull, 0xb6d99325e2b6b654ull, 0x0840303820080828ull, 0xf3cb10e3ebf3f318ull,
        0xae19c36d82aeae2cull, 0xbe99a31dc2bebe7cull, 0x19c8564f6419197dull, 0x893c11981e898997ull,
        0x328dac9ec83232faull, 0x262dd4f2982626beull, 0xb0e98737fab0b04aull, 0xea0346ac8feaea65ull,
        0x4b62a7ec314b4b7aull, 0x640745218d6464e9ull, 0x84543fbb2a8484aeull, 0x82642ba9328282b0ull,
        0x6b7f670cb16b6bdaull, 0xf5fb04f1f3f5f506ull, 0x79ef0b72f9797980ull, 0xbf91a51ac6bfbf79ull,
        0x0108060704010105ull, 0x5fc2df80615f5f3eull, 0x758f2356c97575bcull, 0x633f5734916363f2ull,
        0x1bd85a416c1b1b77ull, 0x2305cae98c2323afull, 0x3df58eb3f43d3dc9ull, 0x68676d05bd6868d5ull,
        0x2a4dfcd6a82a2a82ull, 0x650f4326896565ecull, 0xe8134aa287e8e86full, 0x91fc41d07e9191efull,
        0xf6e30ef8fff6f609ull, 0xffab38c7dbffff24ull, 0x13986a794c13135full, 0x58facd957d585825ull,
        0xf1db1cede3f1f112ull, 0x47028fc801474746ull, 0x0a503c36280a0a22ull, 0x7fdf1f60e17f7f9eull,
        0xc566a46133c5c5f6ull, 0xa751f552a6a7a701ull, 0xe76b688fbbe7e75cull, 0x612f5b3a996161f8ull,
        0x5aeac19b755a5a2full, 0x063014121806061eull, 0x460a89cf05464643ull, 0x441a85c10d444449ull,
        0x422a91d315424257ull, 0x0420181c10040414ull, 0xa069e747baa0a01aull, 0xdb96e03b4bdbdb90ull,
        0x39d596afe43939ddull, 0x864433b5228686a4ull, 0x549ae5b14d545419ull, 0xaa39db7192aaaa38ull,
        0x8c140f830a8c8c86ull, 0x34bdb88cd03434e4ull, 0x2115c6e7842121a5ull, 0x8b2c1d96168b8b9dull,
        0xf8932ad2c7f8f83full, 0x0c602824300c0c3cull, 0x74872551cd7474b9ull, 0x671f4f28816767e6ull
      },
      {
        0x676d05bd6868d568ull, 0x1c09840e8d8d838dull, 0x1e864c0fcacac5caull, 0x52b3fe294d4d644dull,
        0xbf3744d17373a273ull, 0x62a7ec314b4b7a4bull, 0x4ab9f7254e4e6b4eull, 0x4dfcd6a82a2a822aull,
        0xeec21677d4d4a3d4ull, 0xaaf1a35552520752ull, 0x2dd4f2982626be26ull, 0xf18d3ef6b3b345b3ull,
        0x9ae5b14d54541954ull, 0xf0445a781e1e661eull, 0xc8564f6419197d19ull, 0xf8425d7c1f1f631full,
        0x0dccee882222aa22ull, 0x180a090c03030f03ull, 0x0a89cf0546464346ull, 0xf58eb3f43d3dc93dull,
        0x75eec3b42d2d992dull, 0x6aa1eb354a4a7f4aull, 0xa2f7a45153530253ull, 0x6c2dae368383b583ull,
        0x986a794c13135f13ull, 0x241b91128a8a988aull, 0xd19522e6b7b751b7ull, 0xe6c41173d5d5a6d5ull,
        0x35defb942525b125ull, 0xef0b72f979798079ull, 0xfb04f1f3f5f506f5ull, 0x81a914cebdbd73bdull,
        0xfacd957d58582558ull, 0x65e2cdbc2f2f932full, 0x682e23340d0d390dull, 0x100c0e0802020a02ull,
        0x3b54b993eded7eedull, 0xb2fbaa5951510851ull, 0x8463fd429e9edc9eull, 0x8866774411115511ull,
        0xc316e4eff2f21df2ull, 0xed84baf83e3ec63eull, 0x92e3b64955551c55ull, 0xcad987655e5e3b5eull,
        0xc6dc0d63d1d1b2d1ull, 0xb074625816164e16ull, 0xfd88b4f03c3ccc3cull, 0x17492f856666e366ull,
        0xa73d4ddd7070ad70ull, 0xd2d38e695d5d345dull, 0xcb10e3ebf3f318f3ull, 0x1283c60945454c45ull,
        0x3a9ddd1d40405d40ull, 0x2e925e17ccccdbccull, 0x134aa287e8e86fe8ull, 0xd45fcb6a9494fe94ull,
        0x8ae9bf4556561356ull, 0x4030382008082808ull, 0x3e9e501fceced1ceull, 0xd05c46681a1a721aull,
        0xcd9ca6e83a3ad23aull, 0xded6046fd2d2bdd2ull, 0x5b7c9da3e1e142e1ull, 0xb6f8275bdfdf84dfull,
        0xc1992ceeb5b55bb5ull, 0xdd90a8e03838d838ull, 0x577917a56e6ecb6eull, 0x70242a380e0e360eull,
        0x7b6481b3e5e556e5ull, 0xf302f6f7f4f403f4ull, 0x9b2cd5c3f9f93af9ull, 0x4433b5228686a486ull,
        0x1b4ca583e9e96ae9ull, 0x42bff0214f4f6e4full, 0xfece187fd6d6a9d6ull, 0x5c39bc2e8585ab85ull,
        0x05cae98c2323af23ull, 0x3698571bcfcfd4cfull, 0x8dac9ec83232fa32ull, 0xbc71e85e9999c799ull,
        0x95a697c43131f531ull, 0xa0786c5014144414ull, 0x19c36d82aeae2caeull, 0x235eb09feeee71eeull,
        0x0e8a4207c8c8cfc8ull, 0x7aade53d48487548ull, 0xd6d0036bd3d3b8d3ull, 0x9da090c03030f030ull,
        0x61e140bea1a11fa1ull, 0xe44bd9729292e092ull, 0x329bda1941415841ull, 0xe18130feb1b14fb1ull,
        0xc050486018187818ull, 0x6ea26637c4c4f3c4ull, 0x7de8c4b02c2c9c2cull, 0xaf3b4ad97171a871ull,
        0xb73143d57272a772ull, 0x1a85c10d44444944ull, 0xa87e6b5415154115ull, 0xbb34c9d3fdfd2efdull,
        0xa5b285dc3737eb37ull, 0x99a31dc2bebe7cbeull, 0xc2df80615f5f3e5full, 0x39db7192aaaa38aaull,
        0xac7de6569b9bcd9bull, 0x34179f1a88889288ull, 0x8eea3247d8d89fd8ull, 0x31dd7696abab3dabull,
        0x3c11981e89899789ull, 0x946ff34a9c9cd69cull, 0x8326dccffafa35faull, 0x275d3d9d6060fd60ull,
        0x0346ac8feaea65eaull, 0x89af13cabcbc76bcull, 0x375133956262f762ull, 0x602824300c0c3c0cull,
        0x3dd8fc902424b424ull, 0x59f355a2a6a604a6ull, 0x29d77f9aa8a832a8ull, 0x3352be97ecec7becull,
        0x1f4f28816767e667ull, 0x1dc0e0802020a020ull, 0x96e03b4bdbdb90dbull, 0xc71569ed7c7c917cull,
        0x5df0d8a028288828ull, 0xa6f42953dddd8eddull, 0x09cf638aacac26acull, 0xe2c79c715b5b2a5bull,
        0xbdb88cd03434e434ull, 0xd71967e57e7e9b7eull, 0x8060704010105010ull, 0xdb1cede3f1f112f1ull,
        0xff077cf17b7b8a7bull, 0x0c058a068f8f898full, 0x3f5734916363f263ull, 0x69e747baa0a01aa0ull,
        0x281e1b1405051105ull, 0xa47be1529a9ac89aull, 0x2297d41143435243ull, 0x9f2f58c17777b677ull,
        0x15c6e7842121a521ull, 0x91a51ac6bfbf79bfull, 0x25d2f59c2727bb27ull, 0x48363f2409092d09ull,
        0x56b0732bc3c3e8c3ull, 0x8c65fa469f9fd99full, 0xd99325e2b6b654b6ull, 0xf6c81f7bd7d7acd7ull,
        0x55f6dfa429298d29ull, 0x5eb6742fc2c2edc2ull, 0x0b40ab8bebeb60ebull, 0x4eba7a27c0c0e7c0ull,
        0x49ff5baaa4a40ea4ull, 0x2c1d96168b8b9d8bull, 0x140f830a8c8c868cull, 0xe84e53741d1d691dull,
        0x8b20dbcbfbfb30fbull, 0xab38c7dbffff24ffull, 0x46bc7d23c1c1e2c1ull, 0xf98b39f2b2b240b2ull,
        0xcc55c2669797f197ull, 0x6de4cab82e2e962eull, 0x932ad2c7f8f83ff8ull, 0x0f4326896565ec65ull,
        0xe30ef8fff6f609f6ull, 0x8f2356c97575bc75ull, 0x3812151c07071b07ull, 0x20181c1004041404ull,
        0x72abe23949497049ull, 0x85aa99cc3333ff33ull, 0x736286b7e4e453e4ull, 0x86ec3543d9d99ad9ull,
        0xa1b108deb9b967b9ull, 0xceda0a67d0d0b7d0ull, 0x2a91d31542425742ull, 0x76a86f3bc7c7fcc7ull,
        0x477519ad6c6cc16cull, 0xf447d77a9090ea90ull, 0x0000000000000000ull, 0x04038d028e8e8c8eull,
        0x5f7f10a16f6fce6full, 0xbafdad5d50500d50ull, 0x0806070401010501ull, 0x66a46133c5c5f6c5ull,
        0x9ee63c4fdada95daull, 0x028fc80147474647ull, 0xe582bdfc3f3fc33full, 0x26945913cdcddecdull,
        0x6f6b02b96969d069ull, 0x79eb49b2a2a210a2ull, 0x437694afe2e24de2ull, 0xf7017bf57a7a8f7aull,
        0x51f552a6a7a701a7ull, 0x7eae683fc6c6f9c6ull, 0xec4dde769393e593ull, 0x78222d3c0f0f330full,
        0x503c36280a0a220aull, 0x3014121806061e06ull, 0x636e88bfe6e659e6ull, 0x45fad1ac2b2b872bull,
        0xc453c5629696f496ull, 0x71ed4eb6a3a315a3ull, 0xe04854701c1c6c1cull, 0x11c56a86afaf29afull,
        0x77610bb56a6adf6aull, 0x906c7e4812125a12ull, 0x543fbb2a8484ae84ull, 0xd596afe43939dd39ull,
        0x6b688fbbe7e75ce7ull, 0xe98737fab0b04ab0ull, 0x642ba9328282b082ull, 0xeb08fffbf7f70cf7ull,
        0xa33ec0dffefe21feull, 0x9c69f44e9d9dd39dull, 0x4c35b2268787a187ull, 0xdad5896d5c5c315cull,
        0x7c21a03e8181bf81ull, 0xb5be8bd43535e135ull, 0xbefe205fdede81deull, 0xc99f2beab4b45eb4ull,
        0x41f95caea5a50ba5ull, 0xb332ced7fcfc2bfcull, 0x7427a73a8080ba80ull, 0x2b58b79befef74efull,
        0x16804b0bcbcbc0cbull, 0xb1bd06d6bbbb6dbbull, 0x7f670cb16b6bda6bull, 0x97295fc57676b376ull,
        0xb9bb01d2baba68baull, 0xeac19b755a5a2f5aull, 0xcf136ee97d7d947dull, 0xe70d75fd78788578ull,
        0x583a312c0b0b270bull, 0xdc59cc6e9595fb95ull, 0x4b7093abe3e348e3ull, 0x01c9648eadad23adull,
        0x872551cd7474b974ull, 0xb477ef5a9898c298ull, 0xc59aa1ec3b3bd73bull, 0xadb482d83636ee36ull,
        0x0745218d6464e964ull, 0x4f731ea96d6dc46dull, 0xaef22e57dcdc8bdcull, 0xd31aeae7f0f017f0ull,
        0xf2cb927959592059ull, 0x21d1789ea9a937a9ull, 0x5ab5f92d4c4c614cull, 0xb872655c17174b17ull,
        0xdf1f60e17f7f9e7full, 0xfc41d07e9191ef91ull, 0xa9b70fdab8b862b8ull, 0x068c4503c9c9cac9ull,
        0x82efb84157571657ull, 0xd85a416c1b1b771bull, 0x537a9aa7e0e047e0ull, 0x2f5b3a996161f861ull
      },
      {
        0xd77f9aa8a832a829ull, 0x97d4114343524322ull, 0xdf80615f5f3e5fc2ull, 0x14121806061e0630ull,
        0x670cb16b6bda6b7full, 0x2356c97575bc758full, 0x7519ad6c6cc16c47ull, 0xcb927959592059f2ull,
        0x3b4ad97171a871afull, 0xf8275bdfdf84dfb6ull, 0x35b2268787a1874cull, 0x59cc6e9595fb95dcull,
        0x72655c17174b17b8ull, 0x1aeae7f0f017f0d3ull, 0xea3247d8d89fd88eull, 0x363f2409092d0948ull,
        0x731ea96d6dc46d4full, 0x10e3ebf3f318f3cbull, 0x4e53741d1d691de8ull, 0x804b0bcbcbc0cb16ull,
        0x8c4503c9c9cac906ull, 0xb3fe294d4d644d52ull, 0xe8c4b02c2c9c2c7dull, 0xc56a86afaf29af11ull,
        0x0b72f979798079efull, 0x7a9aa7e0e047e053ull, 0x55c2669797f197ccull, 0x34c9d3fdfd2efdbbull,
        0x7f10a16f6fce6f5full, 0xa7ec314b4b7a4b62ull, 0x83c60945454c4512ull, 0x96afe43939dd39d5ull,
        0x84baf83e3ec63eedull, 0xf42953dddd8edda6ull, 0xed4eb6a3a315a371ull, 0xbff0214f4f6e4f42ull,
        0x9f2beab4b45eb4c9ull, 0x9325e2b6b654b6d9ull, 0x7be1529a9ac89aa4ull, 0x242a380e0e360e70ull,
        0x425d7c1f1f631ff8ull, 0xa51ac6bfbf79bf91ull, 0x7e6b5415154115a8ull, 0x7c9da3e1e142e15bull,
        0xabe2394949704972ull, 0xd6046fd2d2bdd2deull, 0x4dde769393e593ecull, 0xae683fc6c6f9c67eull,
        0x4bd9729292e092e4ull, 0x3143d57272a772b7ull, 0x63fd429e9edc9e84ull, 0x5b3a996161f8612full,
        0xdc0d63d1d1b2d1c6ull, 0x5734916363f2633full, 0x26dccffafa35fa83ull, 0x5eb09feeee71ee23ull,
        0x02f6f7f4f403f4f3ull, 0x564f6419197d19c8ull, 0xc41173d5d5a6d5e6ull, 0xc9648eadad23ad01ull,
        0xcd957d58582558faull, 0xff5baaa4a40ea449ull, 0xbd06d6bbbb6dbbb1ull, 0xe140bea1a11fa161ull,
        0xf22e57dcdc8bdcaeull, 0x16e4eff2f21df2c3ull, 0x2dae368383b5836cull, 0xb285dc3737eb37a5ull,
        0x91d315424257422aull, 0x6286b7e4e453e473ull, 0x017bf57a7a8f7af7ull, 0xac9ec83232fa328dull,
        0x6ff34a9c9cd69c94ull, 0x925e17ccccdbcc2eull, 0xdd7696abab3dab31ull, 0xa1eb354a4a7f4a6aull,
        0x058a068f8f898f0cull, 0x7917a56e6ecb6e57ull, 0x181c100404140420ull, 0xd2f59c2727bb2725ull,
        0xe4cab82e2e962e6dull, 0x688fbbe7e75ce76bull, 0x7694afe2e24de243ull, 0xc19b755a5a2f5aeaull,
        0x53c5629696f496c4ull, 0x74625816164e16b0ull, 0xcae98c2323af2305ull, 0xfad1ac2b2b872b45ull,
        0xb6742fc2c2edc25eull, 0x4326896565ec650full, 0x492f856666e36617ull, 0x222d3c0f0f330f78ull,
        0xaf13cabcbc76bc89ull, 0xd1789ea9a937a921ull, 0x8fc8014747464702ull, 0x9bda194141584132ull,
        0xb88cd03434e434bdull, 0xade53d484875487aull, 0x32ced7fcfc2bfcb3ull, 0x9522e6b7b751b7d1ull,
        0x610bb56a6adf6a77ull, 0x179f1a8888928834ull, 0xf95caea5a50ba541ull, 0xf7a45153530253a2ull,
        0x33b5228686a48644ull, 0x2cd5c3f9f93af99bull, 0xc79c715b5b2a5be2ull, 0xe03b4bdbdb90db96ull,
        0x90a8e03838d838ddull, 0x077cf17b7b8a7bffull, 0xb0732bc3c3e8c356ull, 0x445a781e1e661ef0ull,
        0xccee882222aa220dull, 0xaa99cc3333ff3385ull, 0xd8fc902424b4243dull, 0xf0d8a0282888285dull,
        0xb482d83636ee36adull, 0xa86f3bc7c7fcc776ull, 0x8b39f2b2b240b2f9ull, 0x9aa1ec3b3bd73bc5ull,
        0x038d028e8e8c8e04ull, 0x2f58c17777b6779full, 0xbb01d2baba68bab9ull, 0x04f1f3f5f506f5fbull,
        0x786c5014144414a0ull, 0x65fa469f9fd99f8cull, 0x3038200808280840ull, 0xe3b64955551c5592ull,
        0x7de6569b9bcd9bacull, 0xb5f92d4c4c614c5aull, 0x3ec0dffefe21fea3ull, 0x5d3d9d6060fd6027ull,
        0xd5896d5c5c315cdaull, 0xe63c4fdada95da9eull, 0x50486018187818c0ull, 0x89cf05464643460aull,
        0x945913cdcddecd26ull, 0x136ee97d7d947dcfull, 0xc6e7842121a52115ull, 0x8737fab0b04ab0e9ull,
        0x82bdfc3f3fc33fe5ull, 0x5a416c1b1b771bd8ull, 0x11981e898997893cull, 0x38c7dbffff24ffabull,
        0x40ab8bebeb60eb0bull, 0x3fbb2a8484ae8454ull, 0x6b02b96969d0696full, 0x9ca6e83a3ad23acdull,
        0x69f44e9d9dd39d9cull, 0xc81f7bd7d7acd7f6ull, 0xd0036bd3d3b8d3d6ull, 0x3d4ddd7070ad70a7ull,
        0x4f28816767e6671full, 0x9ddd1d40405d403aull, 0x992ceeb5b55bb5c1ull, 0xfe205fdede81debeull,
        0xd38e695d5d345dd2ull, 0xa090c03030f0309dull, 0x41d07e9191ef91fcull, 0x8130feb1b14fb1e1ull,
        0x0d75fd78788578e7ull, 0x6677441111551188ull, 0x0607040101050108ull, 0x6481b3e5e556e57bull,
        0x0000000000000000ull, 0x6d05bd6868d56867ull, 0x77ef5a9898c298b4ull, 0xe747baa0a01aa069ull,
        0xa46133c5c5f6c566ull, 0x0c0e0802020a0210ull, 0xf355a2a6a604a659ull, 0x2551cd7474b97487ull,
        0xeec3b42d2d992d75ull, 0x3a312c0b0b270b58ull, 0xeb49b2a2a210a279ull, 0x295fc57676b37697ull,
        0x8d3ef6b3b345b3f1ull, 0xa31dc2bebe7cbe99ull, 0x9e501fceced1ce3eull, 0xa914cebdbd73bd81ull,
        0xc36d82aeae2cae19ull, 0x4ca583e9e96ae91bull, 0x1b91128a8a988a24ull, 0xa697c43131f53195ull,
        0x4854701c1c6c1ce0ull, 0x52be97ecec7bec33ull, 0x1cede3f1f112f1dbull, 0x71e85e9999c799bcull,
        0x5fcb6a9494fe94d4ull, 0xdb7192aaaa38aa39ull, 0x0ef8fff6f609f6e3ull, 0xd4f2982626be262dull,
        0xe2cdbc2f2f932f65ull, 0x58b79befef74ef2bull, 0x4aa287e8e86fe813ull, 0x0f830a8c8c868c14ull,
        0xbe8bd43535e135b5ull, 0x0a090c03030f0318ull, 0xc21677d4d4a3d4eeull, 0x1f60e17f7f9e7fdfull,
        0x20dbcbfbfb30fb8bull, 0x1e1b140505110528ull, 0xbc7d23c1c1e2c146ull, 0xd987655e5e3b5ecaull,
        0x47d77a9090ea90f4ull, 0xc0e0802020a0201dull, 0x8eb3f43d3dc93df5ull, 0x2ba9328282b08264ull,
        0x08fffbf7f70cf7ebull, 0x46ac8feaea65ea03ull, 0x3c36280a0a220a50ull, 0x2e23340d0d390d68ull,
        0x1967e57e7e9b7ed7ull, 0x2ad2c7f8f83ff893ull, 0xfdad5d50500d50baull, 0x5c46681a1a721ad0ull,
        0xa26637c4c4f3c46eull, 0x12151c07071b0738ull, 0xefb8415757165782ull, 0xb70fdab8b862b8a9ull,
        0x88b4f03c3ccc3cfdull, 0x5133956262f76237ull, 0x7093abe3e348e34bull, 0x8a4207c8c8cfc80eull,
        0xcf638aacac26ac09ull, 0xf1a35552520752aaull, 0x45218d6464e96407ull, 0x6070401010501080ull,
        0xda0a67d0d0b7d0ceull, 0xec3543d9d99ad986ull, 0x6a794c13135f1398ull, 0x2824300c0c3c0c60ull,
        0x6c7e4812125a1290ull, 0xf6dfa429298d2955ull, 0xfbaa5951510851b2ull, 0xb108deb9b967b9a1ull,
        0x98571bcfcfd4cf36ull, 0xce187fd6d6a9d6feull, 0x3744d17373a273bfull, 0x09840e8d8d838d1cull,
        0x21a03e8181bf817cull, 0xe5b14d545419549aull, 0xba7a27c0c0e7c04eull, 0x54b993eded7eed3bull,
        0xb9f7254e4e6b4e4aull, 0x85c10d444449441aull, 0xf552a6a7a701a751ull, 0xfcd6a82a2a822a4dull,
        0x39bc2e8585ab855cull, 0xdefb942525b12535ull, 0x6e88bfe6e659e663ull, 0x864c0fcacac5ca1eull,
        0x1569ed7c7c917cc7ull, 0x1d96168b8b9d8b2cull, 0xe9bf45565613568aull, 0x27a73a8080ba8074ull
      },
      {
        0x501fceced1ce3e9eull, 0x06d6bbbb6dbbb1bdull, 0xab8bebeb60eb0b40ull, 0xd9729292e092e44bull,
        0xac8feaea65ea0346ull, 0x4b0bcbcbc0cb1680ull, 0x794c13135f13986aull, 0x7d23c1c1e2c146bcull,
        0xa583e9e96ae91b4cull, 0xa6e83a3ad23acd9cull, 0x187fd6d6a9d6feceull, 0x39f2b2b240b2f98bull,
        0x046fd2d2bdd2ded6ull, 0xd77a9090ea90f447ull, 0x655c17174b17b872ull, 0xd2c7f8f83ff8932aull,
        0xd315424257422a91ull, 0x6b5415154115a87eull, 0xbf45565613568ae9ull, 0x2beab4b45eb4c99full,
        0x26896565ec650f43ull, 0x54701c1c6c1ce048ull, 0x9f1a888892883417ull, 0xd411434352432297ull,
        0x6133c5c5f6c566a4ull, 0x896d5c5c315cdad5ull, 0x82d83636ee36adb4ull, 0x01d2baba68bab9bbull,
        0xf1f3f5f506f5fb04ull, 0xb8415757165782efull, 0x28816767e6671f4full, 0x840e8d8d838d1c09ull,
        0x97c43131f53195a6ull, 0xf8fff6f609f6e30eull, 0x218d6464e9640745ull, 0x957d58582558facdull,
        0xfd429e9edc9e8463ull, 0xf6f7f4f403f4f302ull, 0xee882222aa220dccull, 0x7192aaaa38aa39dbull,
        0x56c97575bc758f23ull, 0x2d3c0f0f330f7822ull, 0x0e0802020a02100cull, 0x30feb1b14fb1e181ull,
        0x275bdfdf84dfb6f8ull, 0x1ea96d6dc46d4f73ull, 0x44d17373a273bf37ull, 0xfe294d4d644d52b3ull,
        0x69ed7c7c917cc715ull, 0xf2982626be262dd4ull, 0xcab82e2e962e6de4ull, 0xfffbf7f70cf7eb08ull,
        0x3820080828084030ull, 0x8e695d5d345dd2d3ull, 0xc10d444449441a85ull, 0xbaf83e3ec63eed84ull,
        0xfa469f9fd99f8c65ull, 0x6c5014144414a078ull, 0x4207c8c8cfc80e8aull, 0x6d82aeae2cae19c3ull,
        0xb14d545419549ae5ull, 0x7040101050108060ull, 0x3247d8d89fd88eeaull, 0x13cabcbc76bc89afull,
        0x46681a1a721ad05cull, 0x0cb16b6bda6b7f67ull, 0x02b96969d0696f6bull, 0xe3ebf3f318f3cb10ull,
        0x14cebdbd73bd81a9ull, 0x99cc3333ff3385aaull, 0x7696abab3dab31ddull, 0xdccffafa35fa8326ull,
        0x0d63d1d1b2d1c6dcull, 0xe6569b9bcd9bac7dull, 0x05bd6868d568676dull, 0xf7254e4e6b4e4ab9ull,
        0x625816164e16b074ull, 0xcc6e9595fb95dc59ull, 0xd07e9191ef91fc41ull, 0xb09feeee71ee235eull,
        0xf92d4c4c614c5ab5ull, 0x34916363f2633f57ull, 0x8d028e8e8c8e0403ull, 0x9c715b5b2a5be2c7ull,
        0x5e17ccccdbcc2e92ull, 0xb4f03c3ccc3cfd88ull, 0x4f6419197d19c856ull, 0x40bea1a11fa161e1ull,
        0xa03e8181bf817c21ull, 0xe2394949704972abull, 0x7cf17b7b8a7bff07ull, 0x3543d9d99ad986ecull,
        0x10a16f6fce6f5f7full, 0x85dc3737eb37a5b2ull, 0x3d9d6060fd60275dull, 0x4c0fcacac5ca1e86ull,
        0x8fbbe7e75ce76b68ull, 0xd1ac2b2b872b45faull, 0xe53d484875487aadull, 0xc9d3fdfd2efdbb34ull,
        0xc5629696f496c453ull, 0xc60945454c451283ull, 0xced7fcfc2bfcb332ull, 0xda1941415841329bull,
        0x7e4812125a12906cull, 0x23340d0d390d682eull, 0x72f979798079ef0bull, 0x81b3e5e556e57b64ull,
        0x981e898997893c11ull, 0x830a8c8c868c140full, 0x93abe3e348e34b70ull, 0xe0802020a0201dc0ull,
        0x90c03030f0309da0ull, 0x2e57dcdc8bdcaef2ull, 0x22e6b7b751b7d195ull, 0x19ad6c6cc16c4775ull,
        0xeb354a4a7f4a6aa1ull, 0x2ceeb5b55bb5c199ull, 0xbdfc3f3fc33fe582ull, 0xc2669797f197cc55ull,
        0x1677d4d4a3d4eec2ull, 0x33956262f7623751ull, 0xc3b42d2d992d75eeull, 0x121806061e063014ull,
        0x5baaa4a40ea449ffull, 0x5caea5a50ba541f9ull, 0xae368383b5836c2dull, 0x80615f5f3e5fc2dfull,
        0xd6a82a2a822a4dfcull, 0x3c4fdada95da9ee6ull, 0x4503c9c9cac9068cull, 0x0000000000000000ull,
        0x67e57e7e9b7ed719ull, 0x49b2a2a210a279ebull, 0xb64955551c5592e3ull, 0x1ac6bfbf79bf91a5ull,
        0x7744111155118866ull, 0x1173d5d5a6d5e6c4ull, 0xf34a9c9cd69c946full, 0x571bcfcfd4cf3698ull,
        0x2a380e0e360e7024ull, 0x36280a0a220a503cull, 0xb3f43d3dc93df58eull, 0xaa5951510851b2fbull,
        0x6ee97d7d947dcf13ull, 0xde769393e593ec4dull, 0x416c1b1b771bd85aull, 0xc0dffefe21fea33eull,
        0x6637c4c4f3c46ea2ull, 0xc80147474647028full, 0x3f2409092d094836ull, 0xb5228686a4864433ull,
        0x312c0b0b270b583aull, 0x8a068f8f898f0c05ull, 0xf44e9d9dd39d9c69ull, 0x0bb56a6adf6a7761ull,
        0x151c07071b073812ull, 0x08deb9b967b9a1b1ull, 0x37fab0b04ab0e987ull, 0xef5a9898c298b477ull,
        0x486018187818c050ull, 0x9ec83232fa328dacull, 0x4ad97171a871af3bull, 0xec314b4b7a4b62a7ull,
        0xb79befef74ef2b58ull, 0xa1ec3b3bd73bc59aull, 0x4ddd7070ad70a73dull, 0x47baa0a01aa069e7ull,
        0x86b7e4e453e47362ull, 0xdd1d40405d403a9dull, 0xc7dbffff24ffab38ull, 0x732bc3c3e8c356b0ull,
        0x789ea9a937a921d1ull, 0x88bfe6e659e6636eull, 0x75fd78788578e70dull, 0xd5c3f9f93af99b2cull,
        0x96168b8b9d8b2c1dull, 0xcf05464643460a89ull, 0xa73a8080ba807427ull, 0x5a781e1e661ef044ull,
        0xa8e03838d838dd90ull, 0x9da3e1e142e15b7cull, 0x0fdab8b862b8a9b7ull, 0x7f9aa8a832a829d7ull,
        0x9aa7e0e047e0537aull, 0x24300c0c3c0c6028ull, 0xe98c2323af2305caull, 0x5fc57676b3769729ull,
        0x53741d1d691de84eull, 0xfb942525b12535deull, 0xfc902424b4243dd8ull, 0x1b1405051105281eull,
        0xede3f1f112f1db1cull, 0x17a56e6ecb6e5779ull, 0xcb6a9494fe94d45full, 0xd8a0282888285df0ull,
        0xe1529a9ac89aa47bull, 0xbb2a8484ae84543full, 0xa287e8e86fe8134aull, 0x4eb6a3a315a371edull,
        0xf0214f4f6e4f42bfull, 0x58c17777b6779f2full, 0x036bd3d3b8d3d6d0ull, 0xbc2e8585ab855c39ull,
        0x94afe2e24de24376ull, 0xa35552520752aaf1ull, 0xe4eff2f21df2c316ull, 0xa9328282b082642bull,
        0xad5d50500d50bafdull, 0x7bf57a7a8f7af701ull, 0xcdbc2f2f932f65e2ull, 0x51cd7474b9748725ull,
        0xa45153530253a2f7ull, 0x3ef6b3b345b3f18dull, 0x3a996161f8612f5bull, 0x6a86afaf29af11c5ull,
        0xafe43939dd39d596ull, 0x8bd43535e135b5beull, 0x205fdede81debefeull, 0x5913cdcddecd2694ull,
        0x5d7c1f1f631ff842ull, 0xe85e9999c799bc71ull, 0x638aacac26ac09cfull, 0x648eadad23ad01c9ull,
        0x43d57272a772b731ull, 0xc4b02c2c9c2c7de8ull, 0x2953dddd8edda6f4ull, 0x0a67d0d0b7d0cedaull,
        0xb2268787a1874c35ull, 0x1dc2bebe7cbe99a3ull, 0x87655e5e3b5ecad9ull, 0x55a2a6a604a659f3ull,
        0xbe97ecec7bec3352ull, 0x1c10040414042018ull, 0x683fc6c6f9c67eaeull, 0x090c03030f03180aull,
        0x8cd03434e434bdb8ull, 0xdbcbfbfb30fb8b20ull, 0x3b4bdbdb90db96e0ull, 0x927959592059f2cbull,
        0x25e2b6b654b6d993ull, 0x742fc2c2edc25eb6ull, 0x0704010105010806ull, 0xeae7f0f017f0d31aull,
        0x9b755a5a2f5aeac1ull, 0xb993eded7eed3b54ull, 0x52a6a7a701a751f5ull, 0x2f856666e3661749ull,
        0xe7842121a52115c6ull, 0x60e17f7f9e7fdf1full, 0x91128a8a988a241bull, 0xf59c2727bb2725d2ull,
        0x6f3bc7c7fcc776a8ull, 0x7a27c0c0e7c04ebaull, 0xdfa429298d2955f6ull, 0x1f7bd7d7acd7f6c8ull
      },
      {
        0x769393e593ec4ddeull, 0x43d9d99ad986ec35ull, 0x529a9ac89aa47be1ull, 0xeeb5b55bb5c1992cull,
        0x5a9898c298b477efull, 0x882222aa220dcceeull, 0x0945454c451283c6ull, 0xd7fcfc2bfcb332ceull,
        0xd2baba68bab9bb01ull, 0xb56a6adf6a77610bull, 0x5bdfdf84dfb6f827ull, 0x0802020a02100c0eull,
        0x469f9fd99f8c65faull, 0x57dcdc8bdcaef22eull, 0x5951510851b2fbaaull, 0x7959592059f2cb92ull,
        0x354a4a7f4a6aa1ebull, 0x5c17174b17b87265ull, 0xac2b2b872b45fad1ull, 0x2fc2c2edc25eb674ull,
        0x6a9494fe94d45fcbull, 0xf7f4f403f4f302f6ull, 0xd6bbbb6dbbb1bd06ull, 0xb6a3a315a371ed4eull,
        0x956262f762375133ull, 0xb7e4e453e4736286ull, 0xd97171a871af3b4aull, 0x77d4d4a3d4eec216ull,
        0x13cdcddecd269459ull, 0xdd7070ad70a73d4dull, 0x5816164e16b07462ull, 0xa3e1e142e15b7c9dull,
        0x394949704972abe2ull, 0xf03c3ccc3cfd88b4ull, 0x27c0c0e7c04eba7aull, 0x47d8d89fd88eea32ull,
        0x6d5c5c315cdad589ull, 0x569b9bcd9bac7de6ull, 0x8eadad23ad01c964ull, 0x2e8585ab855c39bcull,
        0x5153530253a2f7a4ull, 0xbea1a11fa161e140ull, 0xf57a7a8f7af7017bull, 0x07c8c8cfc80e8a42ull,
        0xb42d2d992d75eec3ull, 0xa7e0e047e0537a9aull, 0x63d1d1b2d1c6dc0dull, 0xd57272a772b73143ull,
        0xa2a6a604a659f355ull, 0xb02c2c9c2c7de8c4ull, 0x37c4c4f3c46ea266ull, 0xabe3e348e34b7093ull,
        0xc57676b37697295full, 0xfd78788578e70d75ull, 0xe6b7b751b7d19522ull, 0xeab4b45eb4c99f2bull,
        0x2409092d0948363full, 0xec3b3bd73bc59aa1ull, 0x380e0e360e70242aull, 0x1941415841329bdaull,
        0x2d4c4c614c5ab5f9ull, 0x5fdede81debefe20ull, 0xf2b2b240b2f98b39ull, 0x7a9090ea90f447d7ull,
        0x942525b12535defbull, 0xaea5a50ba541f95cull, 0x7bd7d7acd7f6c81full, 0x0c03030f03180a09ull,
        0x4411115511886677ull, 0x0000000000000000ull, 0x2bc3c3e8c356b073ull, 0xb82e2e962e6de4caull,
        0x729292e092e44bd9ull, 0x9befef74ef2b58b7ull, 0x254e4e6b4e4ab9f7ull, 0x4812125a12906c7eull,
        0x4e9d9dd39d9c69f4ull, 0xe97d7d947dcf136eull, 0x0bcbcbc0cb16804bull, 0xd43535e135b5be8bull,
        0x4010105010806070ull, 0x73d5d5a6d5e6c411ull, 0x214f4f6e4f42bff0ull, 0x429e9edc9e8463fdull,
        0x294d4d644d52b3feull, 0x9ea9a937a921d178ull, 0x4955551c5592e3b6ull, 0x3fc6c6f9c67eae68ull,
        0x67d0d0b7d0ceda0aull, 0xf17b7b8a7bff077cull, 0x6018187818c05048ull, 0x669797f197cc55c2ull,
        0x6bd3d3b8d3d6d003ull, 0xd83636ee36adb482ull, 0xbfe6e659e6636e88ull, 0x3d484875487aade5ull,
        0x45565613568ae9bfull, 0x3e8181bf817c21a0ull, 0x068f8f898f0c058aull, 0xc17777b6779f2f58ull,
        0x17ccccdbcc2e925eull, 0x4a9c9cd69c946ff3ull, 0xdeb9b967b9a1b108ull, 0xafe2e24de2437694ull,
        0x8aacac26ac09cf63ull, 0xdab8b862b8a9b70full, 0xbc2f2f932f65e2cdull, 0x5415154115a87e6bull,
        0xaaa4a40ea449ff5bull, 0xed7c7c917cc71569ull, 0x4fdada95da9ee63cull, 0xe03838d838dd90a8ull,
        0x781e1e661ef0445aull, 0x2c0b0b270b583a31ull, 0x1405051105281e1bull, 0x7fd6d6a9d6fece18ull,
        0x5014144414a0786cull, 0xa56e6ecb6e577917ull, 0xad6c6cc16c477519ull, 0xe57e7e9b7ed71967ull,
        0x856666e36617492full, 0xd3fdfd2efdbb34c9ull, 0xfeb1b14fb1e18130ull, 0xb3e5e556e57b6481ull,
        0x9d6060fd60275d3dull, 0x86afaf29af11c56aull, 0x655e5e3b5ecad987ull, 0xcc3333ff3385aa99ull,
        0x268787a1874c35b2ull, 0x03c9c9cac9068c45ull, 0xe7f0f017f0d31aeaull, 0x695d5d345dd2d38eull,
        0xa96d6dc46d4f731eull, 0xfc3f3fc33fe582bdull, 0x1a8888928834179full, 0x0e8d8d838d1c0984ull,
        0x3bc7c7fcc776a86full, 0xfbf7f70cf7eb08ffull, 0x741d1d691de84e53ull, 0x83e9e96ae91b4ca5ull,
        0x97ecec7bec3352beull, 0x93eded7eed3b54b9ull, 0x3a8080ba807427a7ull, 0xa429298d2955f6dfull,
        0x9c2727bb2725d2f5ull, 0x1bcfcfd4cf369857ull, 0x5e9999c799bc71e8ull, 0x9aa8a832a829d77full,
        0x5d50500d50bafdadull, 0x3c0f0f330f78222dull, 0xdc3737eb37a5b285ull, 0x902424b4243dd8fcull,
        0xa0282888285df0d8ull, 0xc03030f0309da090ull, 0x6e9595fb95dc59ccull, 0x6fd2d2bdd2ded604ull,
        0xf83e3ec63eed84baull, 0x715b5b2a5be2c79cull, 0x1d40405d403a9dddull, 0x368383b5836c2daeull,
        0xf6b3b345b3f18d3eull, 0xb96969d0696f6b02ull, 0x415757165782efb8ull, 0x7c1f1f631ff8425dull,
        0x1c07071b07381215ull, 0x701c1c6c1ce04854ull, 0x128a8a988a241b91ull, 0xcabcbc76bc89af13ull,
        0x802020a0201dc0e0ull, 0x8bebeb60eb0b40abull, 0x1fceced1ce3e9e50ull, 0x028e8e8c8e04038dull,
        0x96abab3dab31dd76ull, 0x9feeee71ee235eb0ull, 0xc43131f53195a697ull, 0xb2a2a210a279eb49ull,
        0xd17373a273bf3744ull, 0xc3f9f93af99b2cd5ull, 0x0fcacac5ca1e864cull, 0xe83a3ad23acd9ca6ull,
        0x681a1a721ad05c46ull, 0xcbfbfb30fb8b20dbull, 0x340d0d390d682e23ull, 0x23c1c1e2c146bc7dull,
        0xdffefe21fea33ec0ull, 0xcffafa35fa8326dcull, 0xeff2f21df2c316e4ull, 0xa16f6fce6f5f7f10ull,
        0xcebdbd73bd81a914ull, 0x629696f496c453c5ull, 0x53dddd8edda6f429ull, 0x11434352432297d4ull,
        0x5552520752aaf1a3ull, 0xe2b6b654b6d99325ull, 0x2008082808403038ull, 0xebf3f318f3cb10e3ull,
        0x82aeae2cae19c36dull, 0xc2bebe7cbe99a31dull, 0x6419197d19c8564full, 0x1e898997893c1198ull,
        0xc83232fa328dac9eull, 0x982626be262dd4f2ull, 0xfab0b04ab0e98737ull, 0x8feaea65ea0346acull,
        0x314b4b7a4b62a7ecull, 0x8d6464e964074521ull, 0x2a8484ae84543fbbull, 0x328282b082642ba9ull,
        0xb16b6bda6b7f670cull, 0xf3f5f506f5fb04f1ull, 0xf979798079ef0b72ull, 0xc6bfbf79bf91a51aull,
        0x0401010501080607ull, 0x615f5f3e5fc2df80ull, 0xc97575bc758f2356ull, 0x916363f2633f5734ull,
        0x6c1b1b771bd85a41ull, 0x8c2323af2305cae9ull, 0xf43d3dc93df58eb3ull, 0xbd6868d568676d05ull,
        0xa82a2a822a4dfcd6ull, 0x896565ec650f4326ull, 0x87e8e86fe8134aa2ull, 0x7e9191ef91fc41d0ull,
        0xfff6f609f6e30ef8ull, 0xdbffff24ffab38c7ull, 0x4c13135f13986a79ull, 0x7d58582558facd95ull,
        0xe3f1f112f1db1cedull, 0x0147474647028fc8ull, 0x280a0a220a503c36ull, 0xe17f7f9e7fdf1f60ull,
        0x33c5c5f6c566a461ull, 0xa6a7a701a751f552ull, 0xbbe7e75ce76b688full, 0x996161f8612f5b3aull,
        0x755a5a2f5aeac19bull, 0x1806061e06301412ull, 0x05464643460a89cfull, 0x0d444449441a85c1ull,
        0x15424257422a91d3ull, 0x100404140420181cull, 0xbaa0a01aa069e747ull, 0x4bdbdb90db96e03bull,
        0xe43939dd39d596afull, 0x228686a4864433b5ull, 0x4d545419549ae5b1ull, 0x92aaaa38aa39db71ull,
        0x0a8c8c868c140f83ull, 0xd03434e434bdb88cull, 0x842121a52115c6e7ull, 0x168b8b9d8b2c1d96ull,
        0xc7f8f83ff8932ad2ull, 0x300c0c3c0c602824ull, 0xcd7474b974872551ull, 0x816767e6671f4f28ull
      },
      {
        0x6868d568676d05bdull, 0x8d8d838d1c09840eull, 0xcacac5ca1e864c0full, 0x4d4d644d52b3fe29ull,
        0x7373a273bf3744d1ull, 0x4b4b7a4b62a7ec31ull, 0x4e4e6b4e4ab9f725ull, 0x2a2a822a4dfcd6a8ull,
        0xd4d4a3d4eec21677ull, 0x52520752aaf1a355ull, 0x2626be262dd4f298ull, 0xb3b345b3f18d3ef6ull,
        0x545419549ae5b14dull, 0x1e1e661ef0445a78ull, 0x19197d19c8564f64ull, 0x1f1f631ff8425d7cull,
        0x2222aa220dccee88ull, 0x03030f03180a090cull, 0x464643460a89cf05ull, 0x3d3dc93df58eb3f4ull,
        0x2d2d992d75eec3b4ull, 0x4a4a7f4a6aa1eb35ull, 0x53530253a2f7a451ull, 0x8383b5836c2dae36ull,
        0x13135f13986a794cull, 0x8a8a988a241b9112ull, 0xb7b751b7d19522e6ull, 0xd5d5a6d5e6c41173ull,
        0x2525b12535defb94ull, 0x79798079ef0b72f9ull, 0xf5f506f5fb04f1f3ull, 0xbdbd73bd81a914ceull,
        0x58582558facd957dull, 0x2f2f932f65e2cdbcull, 0x0d0d390d682e2334ull, 0x02020a02100c0e08ull,
        0xeded7eed3b54b993ull, 0x51510851b2fbaa59ull, 0x9e9edc9e8463fd42ull, 0x1111551188667744ull,
        0xf2f21df2c316e4efull, 0x3e3ec63eed84baf8ull, 0x55551c5592e3b649ull, 0x5e5e3b5ecad98765ull,
        0xd1d1b2d1c6dc0d63ull, 0x16164e16b0746258ull, 0x3c3ccc3cfd88b4f0ull, 0x6666e36617492f85ull,
        0x7070ad70a73d4dddull, 0x5d5d345dd2d38e69ull, 0xf3f318f3cb10e3ebull, 0x45454c451283c609ull,
        0x40405d403a9ddd1dull, 0xccccdbcc2e925e17ull, 0xe8e86fe8134aa287ull, 0x9494fe94d45fcb6aull,
        0x565613568ae9bf45ull, 0x0808280840303820ull, 0xceced1ce3e9e501full, 0x1a1a721ad05c4668ull,
        0x3a3ad23acd9ca6e8ull, 0xd2d2bdd2ded6046full, 0xe1e142e15b7c9da3ull, 0xdfdf84dfb6f8275bull,
        0xb5b55bb5c1992ceeull, 0x3838d838dd90a8e0ull, 0x6e6ecb6e577917a5ull, 0x0e0e360e70242a38ull,
        0xe5e556e57b6481b3ull, 0xf4f403f4f302f6f7ull, 0xf9f93af99b2cd5c3ull, 0x8686a4864433b522ull,
        0xe9e96ae91b4ca583ull, 0x4f4f6e4f42bff021ull, 0xd6d6a9d6fece187full, 0x8585ab855c39bc2eull,
        0x2323af2305cae98cull, 0xcfcfd4cf3698571bull, 0x3232fa328dac9ec8ull, 0x9999c799bc71e85eull,
        0x3131f53195a697c4ull, 0x14144414a0786c50ull, 0xaeae2cae19c36d82ull, 0xeeee71ee235eb09full,
        0xc8c8cfc80e8a4207ull, 0x484875487aade53dull, 0xd3d3b8d3d6d0036bull, 0x3030f0309da090c0ull,
        0xa1a11fa161e140beull, 0x9292e092e44bd972ull, 0x41415841329bda19ull, 0xb1b14fb1e18130feull,
        0x18187818c0504860ull, 0xc4c4f3c46ea26637ull, 0x2c2c9c2c7de8c4b0ull, 0x7171a871af3b4ad9ull,
        0x7272a772b73143d5ull, 0x444449441a85c10dull, 0x15154115a87e6b54ull, 0xfdfd2efdbb34c9d3ull,
        0x3737eb37a5b285dcull, 0xbebe7cbe99a31dc2ull, 0x5f5f3e5fc2df8061ull, 0xaaaa38aa39db7192ull,
        0x9b9bcd9bac7de656ull, 0x8888928834179f1aull, 0xd8d89fd88eea3247ull, 0xabab3dab31dd7696ull,
        0x898997893c11981eull, 0x9c9cd69c946ff34aull, 0xfafa35fa8326dccfull, 0x6060fd60275d3d9dull,
        0xeaea65ea0346ac8full, 0xbcbc76bc89af13caull, 0x6262f76237513395ull, 0x0c0c3c0c60282430ull,
        0x2424b4243dd8fc90ull, 0xa6a604a659f355a2ull, 0xa8a832a829d77f9aull, 0xecec7bec3352be97ull,
        0x6767e6671f4f2881ull, 0x2020a0201dc0e080ull, 0xdbdb90db96e03b4bull, 0x7c7c917cc71569edull,
        0x282888285df0d8a0ull, 0xdddd8edda6f42953ull, 0xacac26ac09cf638aull, 0x5b5b2a5be2c79c71ull,
        0x3434e434bdb88cd0ull, 0x7e7e9b7ed71967e5ull, 0x1010501080607040ull, 0xf1f112f1db1cede3ull,
        0x7b7b8a7bff077cf1ull, 0x8f8f898f0c058a06ull, 0x6363f2633f573491ull, 0xa0a01aa069e747baull,
        0x05051105281e1b14ull, 0x9a9ac89aa47be152ull, 0x434352432297d411ull, 0x7777b6779f2f58c1ull,
        0x2121a52115c6e784ull, 0xbfbf79bf91a51ac6ull, 0x2727bb2725d2f59cull, 0x09092d0948363f24ull,
        0xc3c3e8c356b0732bull, 0x9f9fd99f8c65fa46ull, 0xb6b654b6d99325e2ull, 0xd7d7acd7f6c81f7bull,
        0x29298d2955f6dfa4ull, 0xc2c2edc25eb6742full, 0xebeb60eb0b40ab8bull, 0xc0c0e7c04eba7a27ull,
        0xa4a40ea449ff5baaull, 0x8b8b9d8b2c1d9616ull, 0x8c8c868c140f830aull, 0x1d1d691de84e5374ull,
        0xfbfb30fb8b20dbcbull, 0xffff24ffab38c7dbull, 0xc1c1e2c146bc7d23ull, 0xb2b240b2f98b39f2ull,
        0x9797f197cc55c266ull, 0x2e2e962e6de4cab8ull, 0xf8f83ff8932ad2c7ull, 0x6565ec650f432689ull,
        0xf6f609f6e30ef8ffull, 0x7575bc758f2356c9ull, 0x07071b073812151cull, 0x0404140420181c10ull,
        0x4949704972abe239ull, 0x3333ff3385aa99ccull, 0xe4e453e4736286b7ull, 0xd9d99ad986ec3543ull,
        0xb9b967b9a1b108deull, 0xd0d0b7d0ceda0a67ull, 0x424257422a91d315ull, 0xc7c7fcc776a86f3bull,
        0x6c6cc16c477519adull, 0x9090ea90f447d77aull, 0x0000000000000000ull, 0x8e8e8c8e04038d02ull,
        0x6f6fce6f5f7f10a1ull, 0x50500d50bafdad5dull, 0x0101050108060704ull, 0xc5c5f6c566a46133ull,
        0xdada95da9ee63c4full, 0x47474647028fc801ull, 0x3f3fc33fe582bdfcull, 0xcdcddecd26945913ull,
        0x6969d0696f6b02b9ull, 0xa2a210a279eb49b2ull, 0xe2e24de2437694afull, 0x7a7a8f7af7017bf5ull,
        0xa7a701a751f552a6ull, 0xc6c6f9c67eae683full, 0x9393e593ec4dde76ull, 0x0f0f330f78222d3cull,
        0x0a0a220a503c3628ull, 0x06061e0630141218ull, 0xe6e659e6636e88bfull, 0x2b2b872b45fad1acull,
        0x9696f496c453c562ull, 0xa3a315a371ed4eb6ull, 0x1c1c6c1ce0485470ull, 0xafaf29af11c56a86ull,
        0x6a6adf6a77610bb5ull, 0x12125a12906c7e48ull, 0x8484ae84543fbb2aull, 0x3939dd39d596afe4ull,
        0xe7e75ce76b688fbbull, 0xb0b04ab0e98737faull, 0x8282b082642ba932ull, 0xf7f70cf7eb08fffbull,
        0xfefe21fea33ec0dfull, 0x9d9dd39d9c69f44eull, 0x8787a1874c35b226ull, 0x5c5c315cdad5896dull,
        0x8181bf817c21a03eull, 0x3535e135b5be8bd4ull, 0xdede81debefe205full, 0xb4b45eb4c99f2beaull,
        0xa5a50ba541f95caeull, 0xfcfc2bfcb332ced7ull, 0x8080ba807427a73aull, 0xefef74ef2b58b79bull,
        0xcbcbc0cb16804b0bull, 0xbbbb6dbbb1bd06d6ull, 0x6b6bda6b7f670cb1ull, 0x7676b37697295fc5ull,
        0xbaba68bab9bb01d2ull, 0x5a5a2f5aeac19b75ull, 0x7d7d947dcf136ee9ull, 0x78788578e70d75fdull,
        0x0b0b270b583a312cull, 0x9595fb95dc59cc6eull, 0xe3e348e34b7093abull, 0xadad23ad01c9648eull,
        0x7474b974872551cdull, 0x9898c298b477ef5aull, 0x3b3bd73bc59aa1ecull, 0x3636ee36adb482d8ull,
        0x6464e9640745218dull, 0x6d6dc46d4f731ea9ull, 0xdcdc8bdcaef22e57ull, 0xf0f017f0d31aeae7ull,
        0x59592059f2cb9279ull, 0xa9a937a921d1789eull, 0x4c4c614c5ab5f92dull, 0x17174b17b872655cull,
        0x7f7f9e7fdf1f60e1ull, 0x9191ef91fc41d07eull, 0xb8b862b8a9b70fdaull, 0xc9c9cac9068c4503ull,
        0x5757165782efb841ull, 0x1b1b771bd85a416cull, 0xe0e047e0537a9aa7ull, 0x6161f8612f5b3a99ull
      }
     }; // kupyna_constants

    namespace kupyna_functions
    {
        template <int R>
        static inline void
        G(uint64_t* x, uint64_t* y)
        {

          for (int c = 0; c != R; ++c)
            y[c] = kupyna_constants<void>::T[0][static_cast<unsigned char>(x[(c + R - 0) % R])]
                 ^ kupyna_constants<void>::T[1][static_cast<unsigned char>(x[(c + R - 1) % R] >>  8)]
                 ^ kupyna_constants<void>::T[2][static_cast<unsigned char>(x[(c + R - 2) % R] >> 16)]
                 ^ kupyna_constants<void>::T[3][static_cast<unsigned char>(x[(c + R - 3) % R] >> 24)]
                 ^ kupyna_constants<void>::T[4][static_cast<unsigned char>(x[(c + R - 4) % R] >> 32)]
                 ^ kupyna_constants<void>::T[5][static_cast<unsigned char>(x[(c + R - 5) % R] >> 40)]
                 ^ kupyna_constants<void>::T[6][static_cast<unsigned char>(x[(c + R - 6) % R] >> 48)]
                 ^ kupyna_constants<void>::T[7][static_cast<unsigned char>(
                                      x[(c + R - (R == 16 ? 11 : 7)) % R] >> 56)
                                               ];
        }

        template <int R>
        static inline void
        roundP(uint64_t* x, uint64_t* y, uint64_t i)
        {
          for (int idx = 0; idx < R; idx++)
            x[idx] ^= (static_cast<uint64_t>(idx) << 4) ^ i;

          G<R>(x, y);
        }

        template <int R>
        static inline void
        roundQ(uint64_t* x, uint64_t* y, uint64_t i)
        {
          for (int j = 0; j < R; ++j)
            x[j] += (0x00F0F0F0F0F0F0F3ULL ^
                   ((static_cast<uint64_t>(((R - 1 - j) * 0x10) ^
                     static_cast<unsigned char>(i))) << 56));

          G<R>(x, y);
        }

        template <int R>
        static inline void
        transform(uint64_t* h, const uint64_t* m)
        {
          uint64_t AQ1[R], AQ2[R], AP1[R], AP2[R];

          for (int column = 0; column < R; column++)
          {
            AP1[column] = h[column] ^ m[column];
            AQ1[column] = m[column];
          }

          for (uint64_t r = 0; r < (R == 16 ? 14 : 10); r += 2)
          {
            roundP<R>(AP1, AP2,  r    );
            roundP<R>(AP2, AP1,  r + 1);
            roundQ<R>(AQ1, AQ2,  r    );
            roundQ<R>(AQ2, AQ1,  r + 1);
          }

          for (int column = 0; column < R; column++)
          {
            h[column] = AP1[column] ^ AQ1[column] ^ h[column];
          }
        }

        template <int R>
        static inline void
        outputTransform(uint64_t* h)
        {
          uint64_t t1[R];
          uint64_t t2[R];

          for (int column = 0; column < R; column++) {
            t1[column] = h[column];
          }

          for (uint64_t r = 0; r < (R == 16 ? 14 : 10); r += 2) {
            roundP<R>(t1, t2, r);
            roundP<R>(t2, t1, r+1);
          }

          for (int column = 0; column < R; column++) {
            h[column] ^= t1[column];
          }
        }

    } // namespace kupyna_functions

    class kupyna_provider
    {
     public:
      static const bool is_xof = false;

      kupyna_provider(size_t hashsize = 512)
      : hs{hashsize}
      {
        validate_hash_size(hashsize, { 256, 512 });
      }

      ~kupyna_provider()
      {
        clear();
      }

      inline void init()
      {
        pos = 0;
        total = 0;
        memset(&h[0], 0, sizeof(uint64_t)*16);
        h[0] = block_bytes(); // state in bytes
      }

      inline void update(const unsigned char* data, size_t len)
      {
        absorb_bytes(data, len,
          block_bytes(), block_bytes(),
          m.data(), pos, total,
          [this](const unsigned char* adata, size_t alen)
                { transform(adata, alen); });
      }

      inline void _final(unsigned char* hash)
      {
        total += pos * 8;
        m[pos++] = 0x80;
        size_t limit = block_bytes();
        if (pos > limit - 12)
        {
          if (limit != pos) memset(&m[pos], 0, limit - pos);
          transform(m.data(), 1);
          total += 0;
          pos = 0;
        }
        memset(&m[pos], 0, limit - pos);
        memcpy(&m[limit - 12], &total, sizeof(uint64_t));
        memset(&m[limit - 4], 0, 4);

        transform(m.data(), 1);
        outputTransform();

        memcpy(hash,
               reinterpret_cast<const unsigned char*>(h.data()) +
               limit - hash_size() / 8,
               hash_size() / 8);
      } // _final

      inline void clear()
      {
        zero_memory(h);
        zero_memory(m);
      }

      inline size_t hash_size() const { return hs; }

     private:

      inline size_t block_bytes() const { return hs > 256 ? 128 : 64; }

      inline void outputTransform()
      {
        if (hs > 256) kupyna_functions::outputTransform<16>(&h[0]);
        else          kupyna_functions::outputTransform< 8>(&h[0]);
      }

      inline void transform(const unsigned char* mp, size_t num_blks)
      {
        for (size_t blk = 0; blk < num_blks; blk++)
        {
          if (hs > 256)
            kupyna_functions::transform<16>(&h[0],
              reinterpret_cast<const uint64_t*>(mp + block_bytes() * blk));
          else
            kupyna_functions::transform< 8>(&h[0],
              reinterpret_cast<const uint64_t*>(mp + block_bytes() * blk));
        }
      }

      std::array<uint64_t, 16> h;
      std::array<unsigned char, 128> m;
      size_t hs;
      size_t pos;
      uint64_t total;

    }; // class kupyna_provider

    template <typename T>
    struct esch_constants
    {
      const static uint32_t C[8];
    };

    template <typename T>
    const uint32_t esch_constants<T>::C[8] =
     {
	0xb7e15162, 0xbf715880, 0x38b4da56, 0x324e7738,
        0xbb1185eb, 0x4f7c7b57, 0xcfbfa1c8, 0xc2b3293d
     }; // esch_constants

    namespace esch_functions
    {

        template <size_t N>
        static inline void
        sparkle(std::array<uint32_t, N>& H, int rounds, int ns)
        {
          for(int s = 0; s < ns; s++) 
          {
            H[1] ^= esch_constants<void>::C[s % 8];
            H[3] ^= s;
            for (int j = 0, px = 0, py = 1; j < rounds; j++, px += 2, py += 2)
            {
              H[px] += rotate_right(H[py], 31);
              H[py] ^= rotate_right(H[px], 24);
              H[px] ^= esch_constants<void>::C[j];
              H[px] += rotate_right(H[py], 17);
              H[py] ^= rotate_right(H[px], 17);
              H[px] ^= esch_constants<void>::C[j];
              H[px] += H[py];
              H[py] ^= rotate_right(H[px], 31);
              H[px] ^= esch_constants<void>::C[j];
              H[px] += rotate_right(H[py], 24);
              H[py] ^= rotate_right(H[px], 16);
              H[px] ^= esch_constants<void>::C[j];
            }
            uint32_t x = H[0] ^ H[2] ^ H[4];
            uint32_t y = H[1] ^ H[3] ^ H[5];
            if (rounds > 6)
            {
              x ^= H[6];
              y ^= H[7];
            }
            x = rotate_right(x ^ (x << 16), 16);
            y = rotate_right(y ^ (y << 16), 16);
            
            for (int i = 0, j = rounds; i < rounds; i+=2, j+=2)
            {
              H[j    ] ^= H[i    ] ^ y;
              H[j + 1] ^= H[i + 1] ^ x;
            }
            x = H[rounds    ];
            y = H[rounds + 1];
            for (int i = 0; i < rounds - 2; i++)
            {
              H[i + rounds] = H[i             ];
              H[i         ] = H[i + rounds + 2];
            }
            H[rounds * 2 - 2] = H[rounds - 2];
            H[rounds * 2 - 1] = H[rounds - 1];
            H[rounds     - 2] = x;
            H[rounds     - 1] = y;
          }
        }

    } // namespace esch_functions

    template <size_t N>
    class esch_provider
    {
     public:
      static const bool is_xof = false;

      template <bool xof=false,
                typename std::enable_if<!xof>::type* = nullptr>
      esch_provider(size_t hashsize)
      : hs{hashsize}, squeezing{false}
      {
        static_assert(N == 384 || N == 512, "Esch only supports 384 and 512 bits state size");
        validate_hash_size(hashsize, {256, 384});
      }

      template <bool xof=false,
                typename std::enable_if<xof>::type* = nullptr>
      esch_provider()
      : hs{N - 128}, squeezing{false}
      {
        static_assert(N == 384 || N == 512, "Esch only supports 384 and 512 bits state size");
      }

      ~esch_provider()
      {
        clear();
      }

      inline void init()
      {
        pos = 0;
        total = 0;
        squeezing = false;
        zero_memory(H);
        zero_memory(m);
      }

      inline void update(const unsigned char* data, size_t len)
      {
        absorb_bytes(data, len,
          16, 16 + 1,
          m.data(), pos, total,
          [this](const unsigned char* adata, size_t alen)
                { transform(adata, alen, false); });
      }

      inline void squeeze(unsigned char* hash, size_t hss)
      {
        size_t processed = 0;
        if (!squeezing)
        {
          total = 1;
          squeezing = true;
          if (pos < 16)
          {
            memset(&m[pos], 0, 16 - pos);
            m[pos] = 0x80;
            H[(hs+128)/64 - 1] ^= (is_xof ? 0x5000000 : 0x1000000);
          }
          else
            H[(hs+128)/64 - 1] ^= (is_xof ? 0x6000000 : 0x2000000);

          transform(m.data(), 1, true);
        }
        else if (pos < 16)
        {
          size_t to_copy = std::min(hss, 16 - pos);
          memcpy(hash, reinterpret_cast<unsigned char*>(H.data()) + pos, to_copy);
          processed += to_copy;
          pos += to_copy;
          total = 0;
        }

        while (processed < hss)
        {
          if (!total)
            esch_functions::sparkle(H, hs > 256 ? 8 : 6, hs > 256 ? 8 : 7);
          pos = std::min(hss - processed, static_cast<size_t>(16));
          memcpy(hash + processed, H.data(), pos);
          processed += pos;
          total = 0;
        }
      }

      inline void _final(unsigned char* hash)
      {
        return squeeze(hash, hs / 8);
      }

      inline void clear()
      {
        zero_memory(H);
        zero_memory(m);
      }

      inline size_t hash_size() const { return hs; }

     private:

      inline void transform(const unsigned char* data,
                            size_t num_blks,
                            bool lastBlock)
      {
        for (size_t blk = 0; blk < num_blks; blk++)
        {
          uint32_t M[4];
          for (int i = 0; i < 4; i++)
            M[i] = reinterpret_cast<const uint32_t*>(data)[blk * 4 + i];
          uint32_t x = M[0] ^ M[2];
          uint32_t y = M[1] ^ M[3];
          x = rotate_right(x ^ (x << 16), 16);
          y = rotate_right(y ^ (y << 16), 16);
          H[0] = H[0] ^ M[0] ^ y;
          H[1] = H[1] ^ M[1] ^ x;
          H[2] = H[2] ^ M[2] ^ y;
          H[3] = H[3] ^ M[3] ^ x;
          H[4] ^= y;
          H[5] ^= x;
          if (hs > 256)
          {
            H[6] ^= y;
            H[7] ^= x;
          }       
          int steps = lastBlock ? 11 : 7;
          if (hs > 256) steps++;
          esch_functions::sparkle(H, hs > 256 ? 8 : 6, steps);
        }
      }

      std::array<uint32_t, N / 32> H;
      std::array<unsigned char, 128> m;
      size_t hs;
      size_t pos;
      uint64_t total;
      bool squeezing;

    }; // class esch_provider


    template <typename V>
    struct echo_constants
    {
      static const uint32_t T[4][256];
    };

    template <typename V>
    const uint32_t echo_constants<V>::T[4][256] =
     {
      {
        0xa56363c6, 0x847c7cf8, 0x997777ee, 0x8d7b7bf6, 0x0df2f2ff, 0xbd6b6bd6, 0xb16f6fde, 0x54c5c591,
        0x50303060, 0x03010102, 0xa96767ce, 0x7d2b2b56, 0x19fefee7, 0x62d7d7b5, 0xe6abab4d, 0x9a7676ec,
        0x45caca8f, 0x9d82821f, 0x40c9c989, 0x877d7dfa, 0x15fafaef, 0xeb5959b2, 0xc947478e, 0x0bf0f0fb,
        0xecadad41, 0x67d4d4b3, 0xfda2a25f, 0xeaafaf45, 0xbf9c9c23, 0xf7a4a453, 0x967272e4, 0x5bc0c09b,
        0xc2b7b775, 0x1cfdfde1, 0xae93933d, 0x6a26264c, 0x5a36366c, 0x413f3f7e, 0x02f7f7f5, 0x4fcccc83,
        0x5c343468, 0xf4a5a551, 0x34e5e5d1, 0x08f1f1f9, 0x937171e2, 0x73d8d8ab, 0x53313162, 0x3f15152a,
        0x0c040408, 0x52c7c795, 0x65232346, 0x5ec3c39d, 0x28181830, 0xa1969637, 0x0f05050a, 0xb59a9a2f,
        0x0907070e, 0x36121224, 0x9b80801b, 0x3de2e2df, 0x26ebebcd, 0x6927274e, 0xcdb2b27f, 0x9f7575ea,
        0x1b090912, 0x9e83831d, 0x742c2c58, 0x2e1a1a34, 0x2d1b1b36, 0xb26e6edc, 0xee5a5ab4, 0xfba0a05b,
        0xf65252a4, 0x4d3b3b76, 0x61d6d6b7, 0xceb3b37d, 0x7b292952, 0x3ee3e3dd, 0x712f2f5e, 0x97848413,
        0xf55353a6, 0x68d1d1b9, 0x00000000, 0x2cededc1, 0x60202040, 0x1ffcfce3, 0xc8b1b179, 0xed5b5bb6,
        0xbe6a6ad4, 0x46cbcb8d, 0xd9bebe67, 0x4b393972, 0xde4a4a94, 0xd44c4c98, 0xe85858b0, 0x4acfcf85,
        0x6bd0d0bb, 0x2aefefc5, 0xe5aaaa4f, 0x16fbfbed, 0xc5434386, 0xd74d4d9a, 0x55333366, 0x94858511,
        0xcf45458a, 0x10f9f9e9, 0x06020204, 0x817f7ffe, 0xf05050a0, 0x443c3c78, 0xba9f9f25, 0xe3a8a84b,
        0xf35151a2, 0xfea3a35d, 0xc0404080, 0x8a8f8f05, 0xad92923f, 0xbc9d9d21, 0x48383870, 0x04f5f5f1,
        0xdfbcbc63, 0xc1b6b677, 0x75dadaaf, 0x63212142, 0x30101020, 0x1affffe5, 0x0ef3f3fd, 0x6dd2d2bf,
        0x4ccdcd81, 0x140c0c18, 0x35131326, 0x2fececc3, 0xe15f5fbe, 0xa2979735, 0xcc444488, 0x3917172e,
        0x57c4c493, 0xf2a7a755, 0x827e7efc, 0x473d3d7a, 0xac6464c8, 0xe75d5dba, 0x2b191932, 0x957373e6,
        0xa06060c0, 0x98818119, 0xd14f4f9e, 0x7fdcdca3, 0x66222244, 0x7e2a2a54, 0xab90903b, 0x8388880b,
        0xca46468c, 0x29eeeec7, 0xd3b8b86b, 0x3c141428, 0x79dedea7, 0xe25e5ebc, 0x1d0b0b16, 0x76dbdbad,
        0x3be0e0db, 0x56323264, 0x4e3a3a74, 0x1e0a0a14, 0xdb494992, 0x0a06060c, 0x6c242448, 0xe45c5cb8,
        0x5dc2c29f, 0x6ed3d3bd, 0xefacac43, 0xa66262c4, 0xa8919139, 0xa4959531, 0x37e4e4d3, 0x8b7979f2,
        0x32e7e7d5, 0x43c8c88b, 0x5937376e, 0xb76d6dda, 0x8c8d8d01, 0x64d5d5b1, 0xd24e4e9c, 0xe0a9a949,
        0xb46c6cd8, 0xfa5656ac, 0x07f4f4f3, 0x25eaeacf, 0xaf6565ca, 0x8e7a7af4, 0xe9aeae47, 0x18080810,
        0xd5baba6f, 0x887878f0, 0x6f25254a, 0x722e2e5c, 0x241c1c38, 0xf1a6a657, 0xc7b4b473, 0x51c6c697,
        0x23e8e8cb, 0x7cdddda1, 0x9c7474e8, 0x211f1f3e, 0xdd4b4b96, 0xdcbdbd61, 0x868b8b0d, 0x858a8a0f,
        0x907070e0, 0x423e3e7c, 0xc4b5b571, 0xaa6666cc, 0xd8484890, 0x05030306, 0x01f6f6f7, 0x120e0e1c,
        0xa36161c2, 0x5f35356a, 0xf95757ae, 0xd0b9b969, 0x91868617, 0x58c1c199, 0x271d1d3a, 0xb99e9e27,
        0x38e1e1d9, 0x13f8f8eb, 0xb398982b, 0x33111122, 0xbb6969d2, 0x70d9d9a9, 0x898e8e07, 0xa7949433,
        0xb69b9b2d, 0x221e1e3c, 0x92878715, 0x20e9e9c9, 0x49cece87, 0xff5555aa, 0x78282850, 0x7adfdfa5,
        0x8f8c8c03, 0xf8a1a159, 0x80898909, 0x170d0d1a, 0xdabfbf65, 0x31e6e6d7, 0xc6424284, 0xb86868d0,
        0xc3414182, 0xb0999929, 0x772d2d5a, 0x110f0f1e, 0xcbb0b07b, 0xfc5454a8, 0xd6bbbb6d, 0x3a16162c
      }, {
        0x6363c6a5, 0x7c7cf884, 0x7777ee99, 0x7b7bf68d, 0xf2f2ff0d, 0x6b6bd6bd, 0x6f6fdeb1, 0xc5c59154,
        0x30306050, 0x01010203, 0x6767cea9, 0x2b2b567d, 0xfefee719, 0xd7d7b562, 0xabab4de6, 0x7676ec9a,
        0xcaca8f45, 0x82821f9d, 0xc9c98940, 0x7d7dfa87, 0xfafaef15, 0x5959b2eb, 0x47478ec9, 0xf0f0fb0b,
        0xadad41ec, 0xd4d4b367, 0xa2a25ffd, 0xafaf45ea, 0x9c9c23bf, 0xa4a453f7, 0x7272e496, 0xc0c09b5b,
        0xb7b775c2, 0xfdfde11c, 0x93933dae, 0x26264c6a, 0x36366c5a, 0x3f3f7e41, 0xf7f7f502, 0xcccc834f,
        0x3434685c, 0xa5a551f4, 0xe5e5d134, 0xf1f1f908, 0x7171e293, 0xd8d8ab73, 0x31316253, 0x15152a3f,
        0x0404080c, 0xc7c79552, 0x23234665, 0xc3c39d5e, 0x18183028, 0x969637a1, 0x05050a0f, 0x9a9a2fb5,
        0x07070e09, 0x12122436, 0x80801b9b, 0xe2e2df3d, 0xebebcd26, 0x27274e69, 0xb2b27fcd, 0x7575ea9f,
        0x0909121b, 0x83831d9e, 0x2c2c5874, 0x1a1a342e, 0x1b1b362d, 0x6e6edcb2, 0x5a5ab4ee, 0xa0a05bfb,
        0x5252a4f6, 0x3b3b764d, 0xd6d6b761, 0xb3b37dce, 0x2929527b, 0xe3e3dd3e, 0x2f2f5e71, 0x84841397,
        0x5353a6f5, 0xd1d1b968, 0x00000000, 0xededc12c, 0x20204060, 0xfcfce31f, 0xb1b179c8, 0x5b5bb6ed,
        0x6a6ad4be, 0xcbcb8d46, 0xbebe67d9, 0x3939724b, 0x4a4a94de, 0x4c4c98d4, 0x5858b0e8, 0xcfcf854a,
        0xd0d0bb6b, 0xefefc52a, 0xaaaa4fe5, 0xfbfbed16, 0x434386c5, 0x4d4d9ad7, 0x33336655, 0x85851194,
        0x45458acf, 0xf9f9e910, 0x02020406, 0x7f7ffe81, 0x5050a0f0, 0x3c3c7844, 0x9f9f25ba, 0xa8a84be3,
        0x5151a2f3, 0xa3a35dfe, 0x404080c0, 0x8f8f058a, 0x92923fad, 0x9d9d21bc, 0x38387048, 0xf5f5f104,
        0xbcbc63df, 0xb6b677c1, 0xdadaaf75, 0x21214263, 0x10102030, 0xffffe51a, 0xf3f3fd0e, 0xd2d2bf6d,
        0xcdcd814c, 0x0c0c1814, 0x13132635, 0xececc32f, 0x5f5fbee1, 0x979735a2, 0x444488cc, 0x17172e39,
        0xc4c49357, 0xa7a755f2, 0x7e7efc82, 0x3d3d7a47, 0x6464c8ac, 0x5d5dbae7, 0x1919322b, 0x7373e695,
        0x6060c0a0, 0x81811998, 0x4f4f9ed1, 0xdcdca37f, 0x22224466, 0x2a2a547e, 0x90903bab, 0x88880b83,
        0x46468cca, 0xeeeec729, 0xb8b86bd3, 0x1414283c, 0xdedea779, 0x5e5ebce2, 0x0b0b161d, 0xdbdbad76,
        0xe0e0db3b, 0x32326456, 0x3a3a744e, 0x0a0a141e, 0x494992db, 0x06060c0a, 0x2424486c, 0x5c5cb8e4,
        0xc2c29f5d, 0xd3d3bd6e, 0xacac43ef, 0x6262c4a6, 0x919139a8, 0x959531a4, 0xe4e4d337, 0x7979f28b,
        0xe7e7d532, 0xc8c88b43, 0x37376e59, 0x6d6ddab7, 0x8d8d018c, 0xd5d5b164, 0x4e4e9cd2, 0xa9a949e0,
        0x6c6cd8b4, 0x5656acfa, 0xf4f4f307, 0xeaeacf25, 0x6565caaf, 0x7a7af48e, 0xaeae47e9, 0x08081018,
        0xbaba6fd5, 0x7878f088, 0x25254a6f, 0x2e2e5c72, 0x1c1c3824, 0xa6a657f1, 0xb4b473c7, 0xc6c69751,
        0xe8e8cb23, 0xdddda17c, 0x7474e89c, 0x1f1f3e21, 0x4b4b96dd, 0xbdbd61dc, 0x8b8b0d86, 0x8a8a0f85,
        0x7070e090, 0x3e3e7c42, 0xb5b571c4, 0x6666ccaa, 0x484890d8, 0x03030605, 0xf6f6f701, 0x0e0e1c12,
        0x6161c2a3, 0x35356a5f, 0x5757aef9, 0xb9b969d0, 0x86861791, 0xc1c19958, 0x1d1d3a27, 0x9e9e27b9,
        0xe1e1d938, 0xf8f8eb13, 0x98982bb3, 0x11112233, 0x6969d2bb, 0xd9d9a970, 0x8e8e0789, 0x949433a7,
        0x9b9b2db6, 0x1e1e3c22, 0x87871592, 0xe9e9c920, 0xcece8749, 0x5555aaff, 0x28285078, 0xdfdfa57a,
        0x8c8c038f, 0xa1a159f8, 0x89890980, 0x0d0d1a17, 0xbfbf65da, 0xe6e6d731, 0x424284c6, 0x6868d0b8,
        0x414182c3, 0x999929b0, 0x2d2d5a77, 0x0f0f1e11, 0xb0b07bcb, 0x5454a8fc, 0xbbbb6dd6, 0x16162c3a
      }, {
        0x63c6a563, 0x7cf8847c, 0x77ee9977, 0x7bf68d7b, 0xf2ff0df2, 0x6bd6bd6b, 0x6fdeb16f, 0xc59154c5,
        0x30605030, 0x01020301, 0x67cea967, 0x2b567d2b, 0xfee719fe, 0xd7b562d7, 0xab4de6ab, 0x76ec9a76,
        0xca8f45ca, 0x821f9d82, 0xc98940c9, 0x7dfa877d, 0xfaef15fa, 0x59b2eb59, 0x478ec947, 0xf0fb0bf0,
        0xad41ecad, 0xd4b367d4, 0xa25ffda2, 0xaf45eaaf, 0x9c23bf9c, 0xa453f7a4, 0x72e49672, 0xc09b5bc0,
        0xb775c2b7, 0xfde11cfd, 0x933dae93, 0x264c6a26, 0x366c5a36, 0x3f7e413f, 0xf7f502f7, 0xcc834fcc,
        0x34685c34, 0xa551f4a5, 0xe5d134e5, 0xf1f908f1, 0x71e29371, 0xd8ab73d8, 0x31625331, 0x152a3f15,
        0x04080c04, 0xc79552c7, 0x23466523, 0xc39d5ec3, 0x18302818, 0x9637a196, 0x050a0f05, 0x9a2fb59a,
        0x070e0907, 0x12243612, 0x801b9b80, 0xe2df3de2, 0xebcd26eb, 0x274e6927, 0xb27fcdb2, 0x75ea9f75,
        0x09121b09, 0x831d9e83, 0x2c58742c, 0x1a342e1a, 0x1b362d1b, 0x6edcb26e, 0x5ab4ee5a, 0xa05bfba0,
        0x52a4f652, 0x3b764d3b, 0xd6b761d6, 0xb37dceb3, 0x29527b29, 0xe3dd3ee3, 0x2f5e712f, 0x84139784,
        0x53a6f553, 0xd1b968d1, 0x00000000, 0xedc12ced, 0x20406020, 0xfce31ffc, 0xb179c8b1, 0x5bb6ed5b,
        0x6ad4be6a, 0xcb8d46cb, 0xbe67d9be, 0x39724b39, 0x4a94de4a, 0x4c98d44c, 0x58b0e858, 0xcf854acf,
        0xd0bb6bd0, 0xefc52aef, 0xaa4fe5aa, 0xfbed16fb, 0x4386c543, 0x4d9ad74d, 0x33665533, 0x85119485,
        0x458acf45, 0xf9e910f9, 0x02040602, 0x7ffe817f, 0x50a0f050, 0x3c78443c, 0x9f25ba9f, 0xa84be3a8,
        0x51a2f351, 0xa35dfea3, 0x4080c040, 0x8f058a8f, 0x923fad92, 0x9d21bc9d, 0x38704838, 0xf5f104f5,
        0xbc63dfbc, 0xb677c1b6, 0xdaaf75da, 0x21426321, 0x10203010, 0xffe51aff, 0xf3fd0ef3, 0xd2bf6dd2,
        0xcd814ccd, 0x0c18140c, 0x13263513, 0xecc32fec, 0x5fbee15f, 0x9735a297, 0x4488cc44, 0x172e3917,
        0xc49357c4, 0xa755f2a7, 0x7efc827e, 0x3d7a473d, 0x64c8ac64, 0x5dbae75d, 0x19322b19, 0x73e69573,
        0x60c0a060, 0x81199881, 0x4f9ed14f, 0xdca37fdc, 0x22446622, 0x2a547e2a, 0x903bab90, 0x880b8388,
        0x468cca46, 0xeec729ee, 0xb86bd3b8, 0x14283c14, 0xdea779de, 0x5ebce25e, 0x0b161d0b, 0xdbad76db,
        0xe0db3be0, 0x32645632, 0x3a744e3a, 0x0a141e0a, 0x4992db49, 0x060c0a06, 0x24486c24, 0x5cb8e45c,
        0xc29f5dc2, 0xd3bd6ed3, 0xac43efac, 0x62c4a662, 0x9139a891, 0x9531a495, 0xe4d337e4, 0x79f28b79,
        0xe7d532e7, 0xc88b43c8, 0x376e5937, 0x6ddab76d, 0x8d018c8d, 0xd5b164d5, 0x4e9cd24e, 0xa949e0a9,
        0x6cd8b46c, 0x56acfa56, 0xf4f307f4, 0xeacf25ea, 0x65caaf65, 0x7af48e7a, 0xae47e9ae, 0x08101808,
        0xba6fd5ba, 0x78f08878, 0x254a6f25, 0x2e5c722e, 0x1c38241c, 0xa657f1a6, 0xb473c7b4, 0xc69751c6,
        0xe8cb23e8, 0xdda17cdd, 0x74e89c74, 0x1f3e211f, 0x4b96dd4b, 0xbd61dcbd, 0x8b0d868b, 0x8a0f858a,
        0x70e09070, 0x3e7c423e, 0xb571c4b5, 0x66ccaa66, 0x4890d848, 0x03060503, 0xf6f701f6, 0x0e1c120e,
        0x61c2a361, 0x356a5f35, 0x57aef957, 0xb969d0b9, 0x86179186, 0xc19958c1, 0x1d3a271d, 0x9e27b99e,
        0xe1d938e1, 0xf8eb13f8, 0x982bb398, 0x11223311, 0x69d2bb69, 0xd9a970d9, 0x8e07898e, 0x9433a794,
        0x9b2db69b, 0x1e3c221e, 0x87159287, 0xe9c920e9, 0xce8749ce, 0x55aaff55, 0x28507828, 0xdfa57adf,
        0x8c038f8c, 0xa159f8a1, 0x89098089, 0x0d1a170d, 0xbf65dabf, 0xe6d731e6, 0x4284c642, 0x68d0b868,
        0x4182c341, 0x9929b099, 0x2d5a772d, 0x0f1e110f, 0xb07bcbb0, 0x54a8fc54, 0xbb6dd6bb, 0x162c3a16
      }, {
        0xc6a56363, 0xf8847c7c, 0xee997777, 0xf68d7b7b, 0xff0df2f2, 0xd6bd6b6b, 0xdeb16f6f, 0x9154c5c5,
        0x60503030, 0x02030101, 0xcea96767, 0x567d2b2b, 0xe719fefe, 0xb562d7d7, 0x4de6abab, 0xec9a7676,
        0x8f45caca, 0x1f9d8282, 0x8940c9c9, 0xfa877d7d, 0xef15fafa, 0xb2eb5959, 0x8ec94747, 0xfb0bf0f0,
        0x41ecadad, 0xb367d4d4, 0x5ffda2a2, 0x45eaafaf, 0x23bf9c9c, 0x53f7a4a4, 0xe4967272, 0x9b5bc0c0,
        0x75c2b7b7, 0xe11cfdfd, 0x3dae9393, 0x4c6a2626, 0x6c5a3636, 0x7e413f3f, 0xf502f7f7, 0x834fcccc,
        0x685c3434, 0x51f4a5a5, 0xd134e5e5, 0xf908f1f1, 0xe2937171, 0xab73d8d8, 0x62533131, 0x2a3f1515,
        0x080c0404, 0x9552c7c7, 0x46652323, 0x9d5ec3c3, 0x30281818, 0x37a19696, 0x0a0f0505, 0x2fb59a9a,
        0x0e090707, 0x24361212, 0x1b9b8080, 0xdf3de2e2, 0xcd26ebeb, 0x4e692727, 0x7fcdb2b2, 0xea9f7575,
        0x121b0909, 0x1d9e8383, 0x58742c2c, 0x342e1a1a, 0x362d1b1b, 0xdcb26e6e, 0xb4ee5a5a, 0x5bfba0a0,
        0xa4f65252, 0x764d3b3b, 0xb761d6d6, 0x7dceb3b3, 0x527b2929, 0xdd3ee3e3, 0x5e712f2f, 0x13978484,
        0xa6f55353, 0xb968d1d1, 0x00000000, 0xc12ceded, 0x40602020, 0xe31ffcfc, 0x79c8b1b1, 0xb6ed5b5b,
        0xd4be6a6a, 0x8d46cbcb, 0x67d9bebe, 0x724b3939, 0x94de4a4a, 0x98d44c4c, 0xb0e85858, 0x854acfcf,
        0xbb6bd0d0, 0xc52aefef, 0x4fe5aaaa, 0xed16fbfb, 0x86c54343, 0x9ad74d4d, 0x66553333, 0x11948585,
        0x8acf4545, 0xe910f9f9, 0x04060202, 0xfe817f7f, 0xa0f05050, 0x78443c3c, 0x25ba9f9f, 0x4be3a8a8,
        0xa2f35151, 0x5dfea3a3, 0x80c04040, 0x058a8f8f, 0x3fad9292, 0x21bc9d9d, 0x70483838, 0xf104f5f5,
        0x63dfbcbc, 0x77c1b6b6, 0xaf75dada, 0x42632121, 0x20301010, 0xe51affff, 0xfd0ef3f3, 0xbf6dd2d2,
        0x814ccdcd, 0x18140c0c, 0x26351313, 0xc32fecec, 0xbee15f5f, 0x35a29797, 0x88cc4444, 0x2e391717,
        0x9357c4c4, 0x55f2a7a7, 0xfc827e7e, 0x7a473d3d, 0xc8ac6464, 0xbae75d5d, 0x322b1919, 0xe6957373,
        0xc0a06060, 0x19988181, 0x9ed14f4f, 0xa37fdcdc, 0x44662222, 0x547e2a2a, 0x3bab9090, 0x0b838888,
        0x8cca4646, 0xc729eeee, 0x6bd3b8b8, 0x283c1414, 0xa779dede, 0xbce25e5e, 0x161d0b0b, 0xad76dbdb,
        0xdb3be0e0, 0x64563232, 0x744e3a3a, 0x141e0a0a, 0x92db4949, 0x0c0a0606, 0x486c2424, 0xb8e45c5c,
        0x9f5dc2c2, 0xbd6ed3d3, 0x43efacac, 0xc4a66262, 0x39a89191, 0x31a49595, 0xd337e4e4, 0xf28b7979,
        0xd532e7e7, 0x8b43c8c8, 0x6e593737, 0xdab76d6d, 0x018c8d8d, 0xb164d5d5, 0x9cd24e4e, 0x49e0a9a9,
        0xd8b46c6c, 0xacfa5656, 0xf307f4f4, 0xcf25eaea, 0xcaaf6565, 0xf48e7a7a, 0x47e9aeae, 0x10180808,
        0x6fd5baba, 0xf0887878, 0x4a6f2525, 0x5c722e2e, 0x38241c1c, 0x57f1a6a6, 0x73c7b4b4, 0x9751c6c6,
        0xcb23e8e8, 0xa17cdddd, 0xe89c7474, 0x3e211f1f, 0x96dd4b4b, 0x61dcbdbd, 0x0d868b8b, 0x0f858a8a,
        0xe0907070, 0x7c423e3e, 0x71c4b5b5, 0xccaa6666, 0x90d84848, 0x06050303, 0xf701f6f6, 0x1c120e0e,
        0xc2a36161, 0x6a5f3535, 0xaef95757, 0x69d0b9b9, 0x17918686, 0x9958c1c1, 0x3a271d1d, 0x27b99e9e,
        0xd938e1e1, 0xeb13f8f8, 0x2bb39898, 0x22331111, 0xd2bb6969, 0xa970d9d9, 0x07898e8e, 0x33a79494,
        0x2db69b9b, 0x3c221e1e, 0x15928787, 0xc920e9e9, 0x8749cece, 0xaaff5555, 0x50782828, 0xa57adfdf,
        0x038f8c8c, 0x59f8a1a1, 0x09808989, 0x1a170d0d, 0x65dabfbf, 0xd731e6e6, 0x84c64242, 0xd0b86868,
        0x82c34141, 0x29b09999, 0x5a772d2d, 0x1e110f0f, 0x7bcbb0b0, 0xa8fc5454, 0x6dd6bbbb, 0x2c3a1616
      }
     }; // echo_constants

    namespace echo_functions
    {
        static inline void shift_rows(uint64_t* w)
        {
          std::swap(w[ 2], w[10]);
          std::swap(w[ 3], w[11]);
          std::swap(w[ 4], w[20]);
          std::swap(w[ 5], w[21]);
          std::swap(w[ 6], w[30]);
          std::swap(w[ 7], w[31]);
          std::swap(w[12], w[28]);
          std::swap(w[13], w[29]);
          std::swap(w[22], w[14]);
          std::swap(w[23], w[15]);
          std::swap(w[30], w[14]);
          std::swap(w[31], w[15]);
          std::swap(w[26], w[18]);
          std::swap(w[27], w[19]);
          std::swap(w[26], w[10]);
          std::swap(w[27], w[11]);
        }

        static inline void mix_columns(uint64_t* w)
        {
          for (int i = 0; i < 4; i++)
          {
            for (int j = 0; j < 2; j++)
            {
              const uint64_t firstbits = 0xfefefefefefefefeull;
              const uint64_t lastbit   = 0x0101010101010101ull;
              size_t idx = i * 4 * 2 + j;
              uint64_t a = w[idx];
              uint64_t b = w[idx + 2];
              uint64_t c = w[idx + 4];
              uint64_t d = w[idx + 6];

              uint64_t dblA = ((a << 1) & firstbits) ^ (((a >> 7) & lastbit) * 0x1b);
              uint64_t dblB = ((b << 1) & firstbits) ^ (((b >> 7) & lastbit) * 0x1b);
              uint64_t dblC = ((c << 1) & firstbits) ^ (((c >> 7) & lastbit) * 0x1b);
              uint64_t dblD = ((d << 1) & firstbits) ^ (((d >> 7) & lastbit) * 0x1b);

              uint64_t WT4 = w[idx] ^ w[idx + 2] ^ w[idx + 4] ^ w[idx + 6];

              w[idx    ] = dblA ^ dblB ^ b ^ c ^ d;
              w[idx + 2] = dblB ^ dblC ^ c ^ d ^ a;
              w[idx + 4] = dblC ^ dblD ^ d ^ a ^ b;
              w[idx + 6] = dblD ^ dblA ^ a ^ b ^ c;
            }
          }
        }

        static inline void sub_words(uint64_t* w, uint64_t* salt, uint64_t& counter)
        {
          for (int r = 0; r < 16; r++)
          {
            size_t idx = r * 2;
            size_t idx1 = r * 2 + 1;
            uint32_t t0 = static_cast<uint32_t>(counter)
                    ^ echo_constants<void>::T[0][(unsigned char)(w[idx ]      )]
                    ^ echo_constants<void>::T[1][(unsigned char)(w[idx ] >> 40)]
                    ^ echo_constants<void>::T[2][(unsigned char)(w[idx1] >> 16)]
                    ^ echo_constants<void>::T[3][(unsigned char)(w[idx1] >> 56)];
            uint32_t t1 = static_cast<uint32_t>(counter >> 32)
                    ^ echo_constants<void>::T[0][(unsigned char)(w[idx ] >> 32)]
                    ^ echo_constants<void>::T[1][(unsigned char)(w[idx1] >>  8)]
                    ^ echo_constants<void>::T[2][(unsigned char)(w[idx1] >> 48)]
                    ^ echo_constants<void>::T[3][(unsigned char)(w[idx ] >> 24)];
            uint32_t t2 = echo_constants<void>::T[0][(unsigned char)(w[idx1])]
                    ^ echo_constants<void>::T[1][(unsigned char)(w[idx1] >> 40)]
                    ^ echo_constants<void>::T[2][(unsigned char)(w[idx ] >> 16)]
                    ^ echo_constants<void>::T[3][(unsigned char)(w[idx ] >> 56)];
            uint32_t t3 = echo_constants<void>::T[0][(unsigned char)(w[idx1] >> 32)]
                    ^ echo_constants<void>::T[1][(unsigned char)(w[idx ] >>  8)]
                    ^ echo_constants<void>::T[2][(unsigned char)(w[idx ] >> 48)]
                    ^ echo_constants<void>::T[3][(unsigned char)(w[idx1] >> 24)];

            ++counter;

            w[idx] = static_cast<uint64_t>(echo_constants<void>::T[0][(unsigned char)(t0)]
                    ^ echo_constants<void>::T[1][(unsigned char)(t1 >>  8)]
                    ^ echo_constants<void>::T[2][(unsigned char)(t2 >> 16)]
                    ^ echo_constants<void>::T[3][(unsigned char)(t3 >> 24)])
                    ^ (
                      static_cast<uint64_t>(echo_constants<void>::T[0][(unsigned char)(t1)]
                              ^ echo_constants<void>::T[1][(unsigned char)(t2 >>  8)]
                              ^ echo_constants<void>::T[2][(unsigned char)(t3 >> 16)]
                              ^ echo_constants<void>::T[3][(unsigned char)(t0 >> 24)])
                      << 32)
                    ^ salt[0];
            w[idx + 1] = static_cast<uint64_t>(echo_constants<void>::T[0][(unsigned char)(t2)]
                    ^ echo_constants<void>::T[1][(unsigned char)(t3 >>  8)]
                    ^ echo_constants<void>::T[2][(unsigned char)(t0 >> 16)]
                    ^ echo_constants<void>::T[3][(unsigned char)(t1 >> 24)])
                    ^ (
                      static_cast<uint64_t>(echo_constants<void>::T[0][(unsigned char)(t3)]
                              ^ echo_constants<void>::T[1][(unsigned char)(t0 >>  8)]
                              ^ echo_constants<void>::T[2][(unsigned char)(t1 >> 16)]
                              ^ echo_constants<void>::T[3][(unsigned char)(t2 >> 24)])
                      << 32)
                    ^ salt[1];
          }
        }


        static inline void final256(uint64_t* h, uint64_t* w)
        {
          h[0] = h[0] ^ h[ 8] ^ h[16] ^ h[24] ^ w[0] ^ w[ 8] ^ w[16] ^ w[24];
          h[1] = h[1] ^ h[ 9] ^ h[17] ^ h[25] ^ w[1] ^ w[ 9] ^ w[17] ^ w[25];
          h[2] = h[2] ^ h[10] ^ h[18] ^ h[26] ^ w[2] ^ w[10] ^ w[18] ^ w[26];
          h[3] = h[3] ^ h[11] ^ h[19] ^ h[27] ^ w[3] ^ w[11] ^ w[19] ^ w[27];
          h[4] = h[4] ^ h[12] ^ h[20] ^ h[28] ^ w[4] ^ w[12] ^ w[20] ^ w[28];
          h[5] = h[5] ^ h[13] ^ h[21] ^ h[29] ^ w[5] ^ w[13] ^ w[21] ^ w[29];
          h[6] = h[6] ^ h[14] ^ h[22] ^ h[30] ^ w[6] ^ w[14] ^ w[22] ^ w[30];
          h[7] = h[7] ^ h[15] ^ h[23] ^ h[31] ^ w[7] ^ w[15] ^ w[23] ^ w[31];
        }

        static inline void final512(uint64_t* h, uint64_t* w)
        {
          h[ 0] = h[ 0] ^ h[16] ^ w[ 0] ^ w[16];
          h[ 1] = h[ 1] ^ h[17] ^ w[ 1] ^ w[17];
          h[ 2] = h[ 2] ^ h[18] ^ w[ 2] ^ w[18];
          h[ 3] = h[ 3] ^ h[19] ^ w[ 3] ^ w[19];
          h[ 4] = h[ 4] ^ h[20] ^ w[ 4] ^ w[20];
          h[ 5] = h[ 5] ^ h[21] ^ w[ 5] ^ w[21];
          h[ 6] = h[ 6] ^ h[22] ^ w[ 6] ^ w[22];
          h[ 7] = h[ 7] ^ h[23] ^ w[ 7] ^ w[23];
          h[ 8] = h[ 8] ^ h[24] ^ w[ 8] ^ w[24];
          h[ 9] = h[ 9] ^ h[25] ^ w[ 9] ^ w[25];
          h[10] = h[10] ^ h[26] ^ w[10] ^ w[26];
          h[11] = h[11] ^ h[27] ^ w[11] ^ w[27];
          h[12] = h[12] ^ h[28] ^ w[12] ^ w[28];
          h[13] = h[13] ^ h[29] ^ w[13] ^ w[29];
          h[14] = h[14] ^ h[30] ^ w[14] ^ w[30];
          h[15] = h[15] ^ h[31] ^ w[15] ^ w[31];
        }

    } // namespace echo_functions

    class echo_provider
    {
     public:
      static const bool is_xof = false;

      echo_provider(size_t hashsize)
      : hs{hashsize}
      {
        validate_hash_size(hashsize, 512);
        zero_memory(salt);
      }

      ~echo_provider()
      {
        clear();
      }

      inline void init()
      {
        pos = 0;
        total = 0;
        memset(&h[0], 0, sizeof(uint64_t)*32);

        for (int i = 0; i < (hs > 256 ? 8 : 4); i++) 
        {
          h[2 * i    ] = hs;
          h[2 * i + 1] = 0;
        }
      }

      inline void set_salt(const unsigned char* asalt, size_t salt_len)
      {
        if (salt_len && salt_len != 16)
          throw std::runtime_error("invalid salt length");

        if (salt_len) memcpy(this->salt.data(), asalt, salt_len);
        else          memset(this->salt.data(), 0, 16);
      }

      inline void update(const unsigned char* data, size_t len)
      {
        absorb_bytes(data, len,
          block_bytes(), block_bytes(),
          reinterpret_cast<unsigned char*>(h.data()) + 256 - block_bytes(), pos, total,
          [this](const unsigned char* adata, size_t num_blks)
                { transform(adata, num_blks, true, block_bytes() * num_blks); });
      }

      inline void _final(unsigned char* hash)
      {
        unsigned char* m = reinterpret_cast<unsigned char*>(h.data()) + 256 - block_bytes();
        total += pos * 8;
        m[pos++] = 0x80;
        size_t limit = block_bytes();
        if (pos > limit - 18)
        {
          if (limit != pos) memset(&m[pos], 0, limit - pos);
          transform(m, 1, true, 0);
          total += 0;
          pos = 0;
        }
        memset(&m[pos], 0, limit - 18 - pos);
        uint16_t hsize = static_cast<uint16_t>(hs);
        memcpy(&m[limit - 18], &hsize, sizeof(hsize));
        memcpy(&m[limit - 16], &total, sizeof(total));
        memset(&m[limit - 8], 0, 8);
        transform(m, 1, pos > 1, 0);

        memcpy(hash, reinterpret_cast<unsigned char*>(h.data()), hash_size() / 8);
      }

      inline void clear()
      {
        zero_memory(h);
        zero_memory(salt);
      }

      inline size_t hash_size() const { return hs; }

     private:

      inline size_t block_bytes() const { return (hs > 256 ? 1024 : 1536) / 8; }

      inline void transform(const unsigned char* mp,
                            size_t num_blks,
                            bool addedbits,
                            uint64_t addtototal)
      {
        unsigned char* m = reinterpret_cast<unsigned char*>(h.data()) + 256 - block_bytes();
        for (size_t blk = 0; blk < num_blks; blk++)
        {
          uint64_t w[32];

          uint64_t counter = 0;
          if (addedbits)
          {
            uint64_t delta = std::min(addtototal, static_cast<uint64_t>(block_bytes()));
            counter = total + (blk * block_bytes() + delta) * 8;
            addtototal -= delta;
            if (m != mp) memcpy(m, mp + block_bytes() * blk, delta);
          }
          memcpy(w, h.data(), sizeof(w));
          int rounds = hs > 256 ? 10 : 8;
          for (int l = 0; l < rounds; l++)
          {
            echo_functions::sub_words(w, salt.data(), counter);
            echo_functions::shift_rows(w);
            echo_functions::mix_columns(w);
          }

          if (hs <= 256) echo_functions::final256(h.data(), w);
          else           echo_functions::final512(h.data(), w);
        }
      }

      std::array<uint64_t, 32> h;
      std::array<uint64_t, 2> salt;
      size_t hs;
      size_t pos;
      uint64_t total;

    }; // class echo_provider

  } // namespace detail


  //////////mixin///////////
  namespace mixin
  {

    template <typename T>
    struct null_mixin
    {
    };

    template <class HashProvider, template <class> class Mixin = null_mixin>
    class hasher; // forward declaration and not the actual definition

    template <typename T>
    class skein_mixin
    {
     public:
      inline hasher<T, mixin::skein_mixin>& 
      set_personalization(const std::string& personalization)
      {
        auto& skein = static_cast<hasher<T, mixin::skein_mixin>&>(*this);
        skein.provider.set_personalization(personalization);
        skein.provider.init();
        return skein;
      }

      template <typename C,
                typename std::enable_if<
                  detail::is_byte<C>::value>::type* = nullptr>
      inline hasher<T, mixin::skein_mixin>&
      set_personalization(const C* personalization, size_t personalization_len)
      {
        auto s = std::string(reinterpret_cast<const char*>(personalization),
                             personalization_len);
        return set_personalization(s);
      }

      inline hasher<T, mixin::skein_mixin>&
      set_key(const std::string& key)
      {
        auto& skein = static_cast<hasher<T, mixin::skein_mixin>&>(*this);
        skein.provider.set_key(key);
        skein.provider.init();
        return skein;
      }

      template <typename C,
                typename std::enable_if<
                  detail::is_byte<C>::value>::type* = nullptr>
      inline hasher<T, mixin::skein_mixin>&
      set_key(const C* key, size_t key_len)
      {
        auto s = std::string(reinterpret_cast<const char*>(key),
                             key_len);
        return set_key(s);
      }

      inline hasher<T, mixin::skein_mixin>&
      set_nonce(const std::string& nonce)
      {
        auto& skein = static_cast<hasher<T, mixin::skein_mixin>&>(*this);
        skein.provider.set_nonce(nonce);
        skein.provider.init();
        return skein;
      }

      template <typename C,
                typename std::enable_if<
                  detail::is_byte<C>::value>::type* = nullptr>
      inline hasher<T, mixin::skein_mixin>&
      set_nonce(const C* nonce, size_t nonce_len)
      {
        auto s = std::string(reinterpret_cast<const char*>(nonce),
                             nonce_len);
        return set_nonce(s);
      }
    };

    template <typename T>
    class echo_mixin
    {
     public:

      inline hasher<T, mixin::echo_mixin>&
      set_salt(const std::string& salt)
      {
        return set_salt(salt.c_str(), salt.size());
      }

      template <typename C,
                typename std::enable_if<
                  detail::is_byte<C>::value>::type* = nullptr>
      inline hasher<T, mixin::echo_mixin>&
      set_salt(const C* salt, size_t salt_len)
      {
        auto& echo = static_cast<hasher<T, mixin::echo_mixin>&>(*this);
        echo.provider.set_salt(reinterpret_cast<const unsigned char*>(salt), salt_len);
        echo.provider.init();
        return echo;
      }
    };

  } // namespace mixin




  template <class HashProvider,
           template <class> class Mixin = mixin::null_mixin>
  class hasher : public Mixin<HashProvider>
  {
   public:
    template <typename H=HashProvider,
              typename std::enable_if<
                std::is_default_constructible<H>::value>::type* = nullptr>
    hasher()
    {
      provider.init();
    }

    template <typename H=HashProvider,
              typename std::enable_if<
                !detail::is_xof<H>::value>::type* = nullptr>
    hasher(size_t hashsize)
    : provider{hashsize}
    {
      provider.init();
    }

    template <typename T,
              typename std::enable_if<
                detail::is_byte<T>::value>::type* = nullptr>
    inline hasher& absorb(const T* data, size_t len)
    {
      provider.update(reinterpret_cast<const unsigned char*>(data), len);
      return *this;
    }

    template <typename T,
              typename std::enable_if<
                detail::is_byte<T>::value &&
                !std::is_same<T,
                 std::string::value_type>::value>::type* = nullptr>
    inline hasher& absorb(const std::basic_string<T>& str)
    {
      if (!str.empty())
        provider.update(reinterpret_cast<const unsigned char*>(&str[0]),
                        str.size());
      return *this;
    }

    inline hasher& absorb(const std::string& str)
    {
      if (!str.empty())
        provider.update(reinterpret_cast<const unsigned char*>(&str[0]),
                        str.size());
      return *this;
    }

    template <typename T,
              typename std::enable_if<
                detail::is_byte<T>::value>::type* = nullptr>
    inline hasher& absorb(std::basic_istream<T>& istr)
    {
      const int tmp_buffer_size = 10000;
      unsigned char buffer[tmp_buffer_size];
      while (istr.read(reinterpret_cast<T*>(buffer), sizeof(buffer)))
        provider.update(buffer, sizeof(buffer));
      size_t gcount = istr.gcount();
      if (gcount) provider.update(buffer, gcount);
      return *this;
    }

    template <typename IT>
    inline hasher& absorb(IT begin, IT end)
    {
      while (begin != end)
      {
        unsigned char byte = *begin++;
        provider.update(&byte, 1);
      }
      return *this;
    }

    template <typename T,
              typename H=HashProvider,
              typename std::enable_if<
                detail::is_byte<T>::value &&
                detail::is_xof<H>::value>::type* = nullptr>
    inline void squeeze(T* buf, size_t len)
    {
      provider.squeeze(reinterpret_cast<unsigned char*>(buf), len);
    }

    template <typename OI,
              typename H=HashProvider,
              typename std::enable_if<
                detail::is_xof<H>::value>::type* = nullptr>
    inline void squeeze(size_t len, OI it)
    {
      std::vector<unsigned char> hash(len);
      provider.squeeze(&hash[0], len);
      std::copy(hash.begin(), hash.end(), it);
    }

    template <typename H=HashProvider,
              typename std::enable_if<
                detail::is_xof<H>::value>::type* = nullptr>
    inline std::string hexsqueeze(size_t len)
    {
      std::ostringstream res;
      res << std::setfill('0') << std::hex;
      squeeze(len, std::ostream_iterator<
        detail::stream_width_fixer<unsigned int, 2>>(res, ""));
      return res.str();
    }

    template <typename T,
              typename H=HashProvider,
              typename std::enable_if<
                detail::is_byte<T>::value &&
                !detail::is_xof<H>::value>::type* = nullptr>
    inline void digest(T* buf, size_t len) const
    {
      if (len < provider.hash_size() / 8)
        throw std::runtime_error("Invalid buffer size");

      HashProvider copy(provider);
      copy._final(buf);
    }

    template <typename OI,
              typename H=HashProvider,
              typename std::enable_if<
                !detail::is_xof<H>::value>::type* = nullptr>
    inline void digest(OI it) const
    {
      HashProvider copy(provider);
      std::vector<unsigned char> hash(provider.hash_size() / 8);
      copy._final(&hash[0]);
      std::copy(hash.begin(), hash.end(), it);
    }

    template <typename H=HashProvider,
              typename std::enable_if<
                !detail::is_xof<H>::value>::type* = nullptr>
    inline std::string hexdigest() const
    {
      std::ostringstream res;
      res << std::setfill('0') << std::hex;
      digest(std::ostream_iterator<
        detail::stream_width_fixer<unsigned int, 2>>(res, ""));
      return res.str();
    }

    inline void reset(bool resetParameters = false)
    {
      if (resetParameters) provider.clear();
      provider.init();
    }

   private:

    friend Mixin<HashProvider>;
    HashProvider provider;
  };

  typedef hasher<detail::skein_provider<1024>, mixin::skein_mixin> skein1024;
  typedef hasher<detail::skein_provider< 512>, mixin::skein_mixin> skein512;
  typedef hasher<detail::skein_provider< 256>, mixin::skein_mixin> skein256;

  typedef hasher<detail::groestl_provider> groestl;

  typedef hasher<detail::jh_provider> jh;

  typedef hasher<detail::kupyna_provider> kupyna;

  typedef hasher<detail::esch_provider<512>> esch;

  typedef hasher<detail::echo_provider, mixin::echo_mixin> echo;

} // namespace digestpp


