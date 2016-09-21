#include <limits>
#include <string>
#include <sstream>
#include <iostream>
#include <cctype>
#include <iomanip>
#include <deque>

template <typename T>
std::string tostr(const T a_value)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(12) << a_value;
    return std::string{"  \tunpacked "} + out.str();
}





std::uint32_t
get0816(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 1234567890123456xxxxxxx
    //                 1234567
    // 16 = 0xFFFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  7) & 0xFFFF;
    g = (uni.dw >> 23) & 0xFF;
    return (g << 16) | r;
}

std::uint32_t
get1815(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 123456789012345xxxxxxxx
    //                12345678
    // 15 = 0x7FFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  8) & 0x7FFF;
    g = (uni.dw >> 23) & 0x1FF;
    return (g << 15) | r;
}

std::uint32_t
get0717(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 12345678901234567xxxxxx
    //                  123456
    // 17 = 0x1FFFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  6) & 0x1FFFF;
    // x.xxxx.xxxx. -> _.x_xx.xxxx. -> 0xxx.xxxx
    g = (uni.dw >> 23) & 0xFF;
    g = ((g & 0x80) >> 1) | (g & 0x3F);
    return (g << 17) | r;
}


std::uint32_t
get1716(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 1234567890123456xxxxxxx
    //                 1234567
    // 16 = 0xFFFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  7) & 0xFFFF;
    // x.xxxx.xxxx. -> x.x_xx.xxxx. -> xxxx.xxxx
    g = (uni.dw >> 23) & 0x1FF;
    g = ((g & 0x180) >> 1) | (g & 0x3F);
    return (g << 16) | r;
}

std::uint32_t
get0618(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 123456789012345678xxxxx
    //                   12345
    // 18 = 0x3FFFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  5) & 0x3FFFF;
    // x.xxxx.xxxx. -> _.x__x.xxxx. -> 00xx.xxxx
    g = (uni.dw >> 23) & 0xFF;
    g = ((g & 0x80) >> 2) | (g & 0x1F);
    return (g << 18) | r;
}

std::uint32_t
get1617(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 12345678901234567xxxxxx
    //                  123456
    // 17 = 0x1FFFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  6) & 0x1FFFF;
    // x.xxxx.xxxx. -> x.x__x.xxxx. -> 0xxx.xxxx
    g = (uni.dw >> 23) & 0x1FF;
    g = ((g & 0x180) >> 2) | (g & 0x1F);
    return (g << 17) | r;
}

std::uint32_t
get0519(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 1234567890123456789xxxx
    //                    1234
    // 19 = 0x7FFFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  4) & 0x7FFFF;
    // x.xxxx.xxxx. -> _.x___.xxxx. -> 000x.xxxx
    g = (uni.dw >> 23) & 0xFF;
    g = ((g & 0x80) >> 3) | (g & 0xF);
    return (g << 19) | r;
}

std::uint32_t
get1518(const float a)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.f = a;
    // 12345678901234567890123
    // 123456789012345678xxxxx
    //                   12345
    // 18 = 0x3FFFF 

    auto r = std::uint32_t{};
    auto g = std::uint32_t{};

    r = (uni.dw >>  5) & 0x3FFFF;
    // x.xxxx.xxxx. -> x.x___.xxxx. -> 00xx.xxxx
    g = (uni.dw >> 23) & 0x1FF;
    g = ((g & 0x180) >> 3) | (g & 0xF);
    return (g << 18) | r;
}

std::uint32_t
get0321(const float a)
{
    // 0000.0000.aaaa.aaaa.bbbb.bbbb.cccc.cXXX   XXX is key for exp
    union Uni
    {
      float f;
      std::uint32_t dw;
    };
    auto uni = Uni{};
    uni.f = a;
    auto exp = std::uint32_t{ (uni.dw >> 23) & 0xFF };
    auto key = std::uint32_t{ exp - 126 };
    uni.dw <<= 1;
    return (uni.dw & 0x00FFFFF8) | (key & 0x7);
}

float
unpack0816(const std::uint32_t f)
{
    // 0x00FFFFFF
    // 23.8.1 float32 = 0.01111001.01000111101011100001010
    // 16.8.0 float24 =   01111001.0100011110101110      

    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw = f << 7;

    return uni.f;
}

float
unpack1815(const std::uint32_t f)
{
    // 0x00FFFFFF
    // 23.8.1 float32 = 0.10000101.11101100011111011111010
    // 15.8.1 float24 = 0.10000101.111011000111110        

    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw = f << 8;

    return uni.f;
}

float 
unpack0717(const std::uint32_t f)
{
    // 0x00FFFFFF
    // 23.8.1 float32 = 0.10001100.10000001110011010110111
    // 17.7.0 float24 =    1001100.10000001110011010      

    // 23.8.1 float32 = 0.01110001.00010111100101111010001
    // 17.7.0 float24 =    0110001.00010111100101111      
 
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw = f << 6;

    // 00xx.xxxx -> 0x?x.xxxx
    // unpacked 0100.xxxx
    //   packed   10.xxxx
    // unpacked 0011.xxxx
    //   packed   01.xxxx
    if (uni.dw  &  0x20000000)
    {
        uni.dw |=  0x40000000;
        uni.dw &= ~0x20000000;
    }
    else
    {
        uni.dw &= ~0x40000000;
        uni.dw |=  0x20000000;
    }

    return uni.f;
}

float
unpack1716(const std::uint32_t f)
{
    // 23.8.1 float32 = 0.10000100.01010010110011000010001
    // 16.7.1 float24 =  0.1000100.0101001011001100       

    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw =   (f <<  7)  & 0x3FFFFFFF;
    uni.dw |= ((f >> 22)  & 0x3) << 30;   // f >> 3byte + f << 4byte

    // 0xxx.xxxx -> xx?x.xxxx
    if (uni.dw  &  0x40000000)
    {
        uni.dw &= ~0x20000000;
    }
    else
    {
        uni.dw |=  0x20000000;
    }

    return uni.f;
}

float
unpack0618(const std::uint32_t f)
{
    // 23.8.1 float32 = 1.10100101.10011101111011100111011
    // 18.6.0 float24 =     100101.100111011110111001     

    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw = f << 5;

    // 00xx.xxxx -> 0x?x.xxxx
    // unpacked 0100.xxxx
    //   packed    1.xxxx
    // unpacked 0011.xxxx
    //   packed    0.xxxx
    if (uni.dw  &  0x10000000)
    {
        uni.dw |=  0x40000000;
        uni.dw &= ~0x20000000;
        uni.dw &= ~0x10000000;
    }
    else
    {
        uni.dw &= ~0x40000000;
        uni.dw |=  0x20000000;
        uni.dw |=  0x10000000;
    }

    return uni.f;
}

float
unpack1617(const std::uint32_t f)
{
    // 23.8.1 float32 = 0.10000101.11101101110100101111001
    // 17.6.1 float24 =   0.100101.11101101110100101     
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw =   (f <<  6)  & 0x3FFFFFFF;
    uni.dw |= ((f >> 22)  & 0x3) << 30;   // f >> 3byte + f << 4byte

    // 0xxx.xxxx -> xx??.xxxx
    if (uni.dw  &  0x40000000)
    {
        uni.dw &= ~0x20000000;
        uni.dw &= ~0x10000000;
    }
    else
    {
        uni.dw |=  0x20000000;
        uni.dw |=  0x10000000;
    }

    return uni.f;
}

float
unpack0519(const std::uint32_t f)
{
    // 23.8.1 float32 = 0.01111110.11000111000111000111001
    // 19.5.0 float24 =      01110.1100011100011100011
    
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw = f << 4;

    // 00xx.xxxx -> 0x?x.xxxx
    // unpacked 0100.xxxx
    //   packed      1xxx
    // unpacked 0011.xxxx
    //   packed      0xxx
    if (uni.dw  &  0x08000000)
    {
        uni.dw |=  0x40000000;
        uni.dw &= ~0x20000000;
        uni.dw &= ~0x10000000;
        uni.dw &= ~0x08000000;
    }
    else
    {
        uni.dw &= ~0x40000000;
        uni.dw |=  0x20000000;
        uni.dw |=  0x10000000;
        uni.dw |=  0x08000000;
    }

    return uni.f;
}

float
unpack1518(const std::uint32_t f)
{
    // 23.8.1 float32 = 1.10000101.11101100011111011111010
    // 18.5.1 float24 =    1.10101.111011000111110111  

    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    auto uni = Uni{};
    uni.dw =   (f <<  5)  & 0x3FFFFFFF;
    uni.dw |= ((f >> 22)  & 0x3) << 30;   // f >> 3byte + f << 4byte

    // 0xxx.xxxx -> xx??.?xxx
    if (uni.dw  &  0x40000000)
    {
        uni.dw &= ~0x20000000;
        uni.dw &= ~0x10000000;
        uni.dw &= ~0x08000000;
    }
    else
    {
        uni.dw |=  0x20000000;
        uni.dw |=  0x10000000;
        uni.dw |=  0x08000000;
    }

    return uni.f;
}

float
unpack0321(const std::uint32_t dw)
{
    // 0000.0000.aaaa.aaaa.bbbb.bbbb.cccc.cXXX   XXX is key for exp
    union Uni
    {
      float f;
      std::uint32_t dw;
    };
    auto uni = Uni{};
    auto key = std::uint32_t{  dw & 0x07 }; 
    auto exp = std::uint32_t{ key + 126  };
    exp      = (exp << 23) & 0x7F800000; 
    uni.dw   = ( dw >>  1) & 0x00FFFFFC;
    uni.dw  |= exp;
    return uni.f;
}




std::string
repr(const float a,
     const int red,
     const int green,
     const int blue)
{
    union Uni
    {
      float f;
      std::uint32_t dw;
    };

    std::deque<char> r;
    std::deque<char> g;
    std::deque<char> b;

    auto uni = Uni{};
    uni.f = a;

    for (int i = 0; i<blue; i++)
    {
        char ch = (uni.dw & 1) ? '1' : '0';
        b.emplace_front(ch);
        uni.dw >>= 1;
    }

    for (int i = 0; i<green; i++)
    {
        char ch = (uni.dw & 1) ? '1' : '0';
        g.emplace_front(ch);
        uni.dw >>= 1;
    }

    for (int i = 0; i<red; i++)
    {
        char ch = (uni.dw & 1) ? '1' : '0';
        r.emplace_front(ch);
        uni.dw >>= 1;
    }

    std::stringstream ss;
    if (red) {

    ss << "\033[1;36m";    for(const auto& bit : r) ss << bit;       ss << "\033[0m";
    ss << "\033[1;97m"     << "."                                       << "\033[0m";   }
    ss << "\033[1;32m";    for(const auto& bit : g) ss << bit;       ss << "\033[0m";
    ss << "\033[1;97m"     << "."                                       << "\033[0m";
    ss << "\033[1;31m";    for(const auto& bit : b) ss << bit;       ss << "\033[0m";

    return ss.str();
}


std::string
repr(const std::uint32_t a,
     const int blue,
     const int green,
     const int red)
{
    std::uint32_t dw = a;

    std::deque<char> r;
    std::deque<char> g;
    std::deque<char> b;

    if (0 == blue && 3 == green && 21 == red)
    {
        for (int i = 0; i<green; i++)
        {
            char ch = (dw & 1) ? '1' : '0';
            g.emplace_front(ch);
            dw >>= 1;
        }
        for (int i = 0; i<red; i++)
        {
            char ch = (dw & 1) ? '1' : '0';
            r.emplace_front(ch);
            dw >>= 1;
        }
    }
    else
    {

    for (int i = 0; i<red; i++)
    {
        char ch = (dw & 1) ? '1' : '0';
        r.emplace_front(ch);
        dw >>= 1;
    }

    for (int i = 0; i<green; i++)
    {
        char ch = (dw & 1) ? '1' : '0';
        g.emplace_front(ch);
        dw >>= 1;
    }

    for (int i = 0; i<blue; i++)
    {
        char ch = (dw & 1) ? '1' : '0';
        b.emplace_front(ch);
        dw >>= 1;
    }
    }
    std::stringstream ss;
    if (blue) {

    ss << "\033[1;36m";    for(const auto& bit : b) ss << bit;       ss << "\033[0m";
    ss << "\033[1;97m"     << "."                                       << "\033[0m";   }
    ss << "\033[1;32m";    for(const auto& bit : g) ss << bit;       ss << "\033[0m";
    ss << "\033[1;97m"     << "."                                       << "\033[0m";
    if (0 == blue && 3 == green && 21 == red) ss << "\033[1;41m"; else
    ss << "\033[1;31m";    for(const auto& bit : r) ss << bit;       ss << "\033[0m";

    return ss.str();
}



std::string
byte2hex (const std::uint8_t b)
{
    std::stringstream ss;
    ss.fill('0');
    ss << std::uppercase << std::setw(2) << std::hex << unsigned(b);
    return ss.str();
}

std::string
hex(const float a)
{
    union Uni
    {
      float f;
      std::uint8_t b[4];
    };
    
    auto uni = Uni{};
    uni.f = a;

    std::stringstream ss;
    ss << "[";
    for (const auto& elem : {uni.b[3], uni.b[2], uni.b[1], uni.b[0]} )  // reverse!
    {
       ss << byte2hex(elem);
       ss << ".";
    }
    ss.seekp(12);
    ss << "]";
    return ss.str();
}

std::string
hex(const std::uint32_t a)
{
    union Uni
    {
      std::uint32_t u;
      std::uint8_t b[4];
    };
    
    auto uni = Uni{};
    uni.u = a;

    std::stringstream ss;
    ss << "[";
    for (const auto& elem : {/*uni.b[3], */uni.b[2], uni.b[1], uni.b[0]} )  // reverse!
    {
       ss << byte2hex(elem);
       ss << ".";
    }
    ss.seekp(/*12*/9);
    ss << "]";
    return ss.str();

}



int main()
{
    float value;
    std::cout << "Hello, enter float value, like 12.34 or 1.23457e+15\n";
    std::cout << "Warning: float min is " << std::numeric_limits<float>::min()
              << " and max is "           << std::numeric_limits<float>::max() << '\n';

    while( !std::cin.eof() ) // to send EOF press Ctrl-D
    {  
       std::cin >> value;
       if ( std::cin.fail() ) 
       {
           break;
       }
       else
       {
           auto float0816 = get0816(value);
           auto float1815 = get1815(value);
           auto float0717 = get0717(value);
           auto float1716 = get1716(value);
           auto float0618 = get0618(value);
           auto float1617 = get1617(value);
           auto float0519 = get0519(value);
           auto float1518 = get1518(value);
           auto float0321 = get0321(value);

           float out0816 = unpack0816(float0816);
           float out1815 = unpack1815(float1815);
           float out0717 = unpack0717(float0717);
           float out1716 = unpack1716(float1716);
           float out0618 = unpack0618(float0618);
           float out1617 = unpack1617(float1617);
           float out0519 = unpack0519(float0519);
           float out1518 = unpack1518(float1518);
           float out0321 = unpack0321(float0321);


           std::cout << "23.8.1 float32 = "        << repr(value,    1,8,23)   << "    "
                     << hex(value)     << tostr(value)   << "  "  << hex(value)   << '\n'
                     << "16.8.0 float24 =   "      << repr(float0816,0,8,16)   << "           "
                     << hex(float0816) << tostr(out0816) << "  "  << hex(out0816) << '\n'
                     << "15.8.1 float24 = "        << repr(float1815,1,8,15)   << "            "
                     << hex(float1815) << tostr(out1815) << "  "  << hex(out1815) << '\n'
                     << "17.7.0 float24 =    "     << repr(float0717,0,7,17)   << "          "
                     << hex(float0717) << tostr(out0717) << "  "  << hex(out0717) << '\n'
                     << "16.7.1 float24 =  "       << repr(float1716,1,7,16)   << "           "
                     << hex(float1716) << tostr(out1716) << "  "  << hex(out1716) << '\n'
                     << "18.6.0 float24 =     "    << repr(float0618,0,6,18)   << "         "
                     << hex(float0618) << tostr(out0618) << "  "  << hex(out0618) << '\n'
                     << "17.6.1 float24 =   "      << repr(float1617,1,6,17)   << "          "
                     << hex(float1617) << tostr(out1617) << "  "  << hex(out1617) << '\n'
                     << "19.5.0 float24 =      "   << repr(float0519,0,5,19)   << "        "
                     << hex(float0519) << tostr(out0519) << "  "  << hex(out0519) << '\n'
                     << "18.5.1 float24 =    "     << repr(float1518,1,5,18)   << "         "
                     << hex(float1518) << tostr(out1518) << "  "  << hex(out1518) << '\n'
                     << "21.3.0 float24 =        " << repr(float0321,0,3,21)   << "      "
                     << hex(float0321) << tostr(out0321) << "  "  << hex(out0321) << std::endl;
       }
    }
}
