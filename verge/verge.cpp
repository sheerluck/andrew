// c++ -O3 -Wall -shared -std=c++23 -fPIC $(python3 -m pybind11 --includes) verge.cpp -o verge$(python3-config --extension-suffix)

#include "bmcufukelushshsi.hpp"
#include <pybind11/pybind11.h>


namespace py = pybind11;


template <unsigned int N>
std::string hexlify(const std::uint8_t hash[N])
{
    std::string hex;
    hex.reserve(N * 2);

    const std::string abc = "0123456789abcdef";
    for (size_t i = 0; i<N; i++)
    {
        const auto c = hash[i];
        hex.push_back(abc[c >> 4]);
        hex.push_back(abc[c & 0x0F]);
    }
    return hex;
}

////////////bmw///////////////////////////////////////////////
std::string bmw256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_bmw_small_context{};
  sph_bmw256_init (&ctx);
  sph_bmw256      (&ctx, bytes, length);
  sph_bmw256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string bmw512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_bmw_big_context{};
  sph_bmw512_init (&ctx);
  sph_bmw512      (&ctx, bytes, length);
  sph_bmw512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}


////////////cubehash//////////////////////////////////////////
std::string cubehash256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_cubehash_context{};
  sph_cubehash256_init (&ctx);
  sph_cubehash256      (&ctx, bytes, length);
  sph_cubehash256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string cubehash512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_cubehash_context{};
  sph_cubehash512_init (&ctx);
  sph_cubehash512      (&ctx, bytes, length);
  sph_cubehash512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}


////////////fugue/////////////////////////////////////////////
std::string fugue256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_fugue_context{};
  sph_fugue256_init (&ctx);
  sph_fugue256      (&ctx, bytes, length);
  sph_fugue256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string fugue512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_fugue_context{};
  sph_fugue512_init (&ctx);
  sph_fugue512      (&ctx, bytes, length);
  sph_fugue512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}


////////////keccak////////////////////////////////////////////
std::string keccak256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_keccak_context{};
  sph_keccak256_init (&ctx);
  sph_keccak256      (&ctx, bytes, length);
  sph_keccak256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string keccak512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_keccak_context{};
  sph_keccak512_init (&ctx);
  sph_keccak512      (&ctx, bytes, length);
  sph_keccak512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}


////////////luffa/////////////////////////////////////////////
std::string luffa256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_luffa256_context{};
  sph_luffa256_init (&ctx);
  sph_luffa256      (&ctx, bytes, length);
  sph_luffa256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string luffa512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_luffa512_context{};
  sph_luffa512_init (&ctx);
  sph_luffa512      (&ctx, bytes, length);
  sph_luffa512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}


////////////shabal////////////////////////////////////////////
std::string shabal256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_shabal_context{};
  sph_shabal256_init (&ctx);
  sph_shabal256      (&ctx, bytes, length);
  sph_shabal256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string shabal512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_shabal_context{};
  sph_shabal512_init (&ctx);
  sph_shabal512      (&ctx, bytes, length);
  sph_shabal512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}


////////////shavite///////////////////////////////////////////
std::string shavite256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_shavite_small_context{};
  sph_shavite256_init (&ctx);
  sph_shavite256      (&ctx, bytes, length);
  sph_shavite256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string shavite512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_shavite_big_context{};
  sph_shavite512_init (&ctx);
  sph_shavite512      (&ctx, bytes, length);
  sph_shavite512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}


////////////simd//////////////////////////////////////////////
std::string simd256(const py::bytes &by)
{
  std::uint8_t hash256[32];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_simd_small_context{};
  sph_simd256_init (&ctx);
  sph_simd256      (&ctx, bytes, length);
  sph_simd256_close(&ctx, hash256);
  return hexlify<32>(hash256);
}


std::string simd512(const py::bytes &by)
{
  std::uint8_t hash512[64];
  auto info = py::buffer{by}.request();
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(info.ptr);
  const auto length = static_cast<size_t>(info.size);
  auto ctx = sph_simd_big_context{};
  sph_simd512_init (&ctx);
  sph_simd512      (&ctx, bytes, length);
  sph_simd512_close(&ctx, hash512);
  return hexlify<64>(hash512);
}





PYBIND11_MODULE(verge, m) {
    m.doc() = "pybind11 plugin";
    m.def("bmw256",      &bmw256,      "bmw256");
    m.def("bmw512",      &bmw512,      "bmw512");
    m.def("cubehash256", &cubehash256, "cubehash256");
    m.def("cubehash512", &cubehash512, "cubehashl512");
    m.def("fugue256",    &fugue256,    "fugue256");
    m.def("fugue512",    &fugue512,    "fugue512");
    m.def("keccak256",   &keccak256,   "keccak256");
    m.def("keccak512",   &keccak512,   "keccak512");
    m.def("luffa256",    &luffa256,    "luffa256");
    m.def("luffa512",    &luffa512,    "luffa512");
    m.def("shabal256",   &shabal256,   "shabal256");
    m.def("shabal512",   &shabal512,   "shabal512");
    m.def("shavite256",  &shavite256,  "shavite256");
    m.def("shavite512",  &shavite512,  "shavite512");
    m.def("simd256",     &simd256,     "simd256");
    m.def("simd512",     &simd512,     "simd512");
}
