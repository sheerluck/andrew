#include "digestpp.hpp"
#include <numeric>
#include <iostream>
#include <iomanip>
#include <fstream>

bool compare(const std::string& name, const std::string& actual, const std::string& expected)
{
        if (actual != expected)
        {
                std::cerr << name << " error: expected " << expected << ", actual " << actual << std::endl;
                return false;
        }
        return true;
}

template<typename H, template<typename> class M, typename std::enable_if<!digestpp::detail::is_xof<H>::value>::type* = nullptr>
std::string get_digest(size_t, digestpp::hasher<H, M>& hasher)
{
        return hasher.hexdigest();
}

template<typename H, template<typename> class M, typename std::enable_if<digestpp::detail::is_xof<H>::value>::type* = nullptr>
std::string get_digest(size_t size, digestpp::hasher<H, M>& hasher)
{
        return hasher.hexsqueeze(size);
}

template<typename H>
bool update_test(const std::string& name, const H& h)
{
        H h1 = h;
        std::array<unsigned char, 256> m;
        std::iota(std::begin(m), std::end(m), 0);

        for (size_t l1 = 1; l1 < m.size() - 1; l1++)
        {
                for (size_t l2 = 1; l2 < m.size() - l1; l2++)
                {
                        H h1 = h;
                        H h2 = h;
                        h1.absorb(m.data(), l1);
                        h1.absorb(m.data() + l1, l2);
                        h2.absorb(m.data(), l1 + l2);
                        std::string s1 = get_digest(32, h1);
                        std::string s2 = get_digest(32, h2);

                        if (s1 != s2)
                        {
                                std::cerr << name << " error: update test failed (l1=" << l1 <<", l2=" << l2 << ')' << std::endl;
                                std::cerr << "s1: " << s1 << std::endl;
                                std::cerr << "s2: " << s2 << std::endl;
                                return false;
                        }
                }
        }
        return true;
}

template<typename XOF>
bool xof_test(const std::string& name, const std::string& ts)
{
        XOF xof1, xof2;
        xof1.absorb(ts);
        xof2.absorb(ts);

        std::string s1 = xof1.hexsqueeze(1000);
        std::string s2;
        for (int i = 0; i != 1000; i++)
                s2 += xof2.hexsqueeze(1);

        if (s1 != s2)
        {
                std::cerr << name << " error: batch squeeze result is not equal to per-byte squeeze" << std::endl;
                std::cerr << "s1: " << s1 << std::endl;
                std::cerr << "s2: " << s2 << std::endl;
                return false;
        }
        return true;
}

void basic_self_test()
{
        std::string ts = "The quick brown fox jumps over the lazy dog";

        int errors = 0;

        errors += !update_test("ECHO/256", digestpp::echo(256));
        errors += !update_test("ECHO/512", digestpp::echo(512));
        errors += !update_test("Esch/256", digestpp::esch(256));
        errors += !update_test("Esch/384", digestpp::esch(384));
        errors += !update_test("Groestl/256", digestpp::groestl(256));
        errors += !update_test("Groestl/512", digestpp::groestl(512));
        errors += !update_test("JH/256", digestpp::jh(256));
        errors += !update_test("JH/512", digestpp::jh(512));
        errors += !update_test("Kupyna/256", digestpp::kupyna(256));
        errors += !update_test("Kupyna/512", digestpp::kupyna(512));
        errors += !update_test("Skein256/256", digestpp::skein256(256));
        errors += !update_test("Skein512/256", digestpp::skein512(256));
        errors += !update_test("Skein512/512", digestpp::skein512(512));
        errors += !update_test("Skein1024/256", digestpp::skein1024(256));
        errors += !update_test("Skein1024/512", digestpp::skein1024(512));
        errors += !update_test("Skein1024/1024", digestpp::skein1024(1024));


        errors += !compare("Groestl/256", digestpp::groestl(256).absorb(ts).hexdigest(),
                "8c7ad62eb26a21297bc39c2d7293b4bd4d3399fa8afab29e970471739e28b301");

        errors += !compare("JH/256", digestpp::jh(256).absorb(ts).hexdigest(),
                "6a049fed5fc6874acfdc4a08b568a4f8cbac27de933496f031015b38961608a0");

        errors += !compare("Kupyna/256", digestpp::kupyna(256).absorb(ts).hexdigest(),
                "996899f2d7422ceaf552475036b2dc120607eff538abf2b8dff471a98a4740c6");

        errors += !compare("Groestl/512", digestpp::groestl(512).absorb(ts).hexdigest(),
                "badc1f70ccd69e0cf3760c3f93884289da84ec13c70b3d12a53a7a8a4a513f99"
                "715d46288f55e1dbf926e6d084a0538e4eebfc91cf2b21452921ccde9131718d");

        errors += !compare("JH/512", digestpp::jh(512).absorb(ts).hexdigest(),
                "043f14e7c0775e7b1ef5ad657b1e858250b21e2e61fd699783f8634cb86f3ff9"
                "38451cabd0c8cdae91d4f659d3f9f6f654f1bfedca117ffba735c15fedda47a3");

        errors += !compare("Kupyna/512", digestpp::kupyna(512).absorb(ts).hexdigest(),
                "d1b469f43e0963735b6cd08a6e75fc370956d8afa9653b5231614decb1cceb81"
                "62ae5b8ff2dc3b02417f86dc4df621d0ca5b1ff399d494766c93a6d2513cae3a");

        errors += !compare("Skein256/512", digestpp::skein256(512).absorb(ts).hexdigest(),
                "f8138e72cdd9e11cf09e4be198c234acb0d21a9f75f936e989cf532f1fa9f4fb"
                "21d255811f0f1592fb3617d04704add875ae7bd16ddbbeaed4eca6eb9675d2c6");

        errors += !compare("Skein512/512", digestpp::skein512(512).absorb(ts).hexdigest(),
                "94c2ae036dba8783d0b3f7d6cc111ff810702f5c77707999be7e1c9486ff238a"
                "7044de734293147359b4ac7e1d09cd247c351d69826b78dcddd951f0ef912713");

        errors += !compare("Skein1024/512", digestpp::skein1024(512).absorb(ts).hexdigest(),
                "a40ba71fa36a8c1d152bfc68b79782ef206d2e74b9a072b11aa874e6ec2148d9"
                "37e9acd4ca1026ad636fed1a88b740112d782e2ca0e6c3bbe0dd2704a60a10a5");


        std::cout << "Self-test completed with " << errors << " errors." << std::endl;
}

int main()
{
  basic_self_test();
}
