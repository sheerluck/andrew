// c++ -O3 -Wall -shared -std=c++23 -fPIC $(python3 -m pybind11 --includes) kerukuro_digestpp.cpp -o kerukuro_digestpp$(python3-config --extension-suffix)

#include "digestpp.hpp"
#include <pybind11/pybind11.h>

#include <numeric>
#include <iostream>
#include <iomanip>
#include <fstream>


std::string skein256(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::skein256(256).absorb(file).hexdigest();
}

std::string skein512(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::skein512(512).absorb(file).hexdigest();
}


std::string groestl256(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::groestl(256).absorb(file).hexdigest();
}

std::string groestl512(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::groestl(512).absorb(file).hexdigest();
}


std::string jh256(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::jh(256).absorb(file).hexdigest();
}

std::string jh512(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::jh(512).absorb(file).hexdigest();
}


std::string kupyna256(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::kupyna(256).absorb(file).hexdigest();
}

std::string kupyna512(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::kupyna(512).absorb(file).hexdigest();
}


std::string esch256(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::esch(256).absorb(file).hexdigest();
}

std::string esch512(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::esch(512).absorb(file).hexdigest();
}


std::string echo256(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::echo(256).absorb(file).hexdigest();
}

std::string echo512(std::string fn)
{
  std::ifstream file(fn, std::ios_base::in|std::ios_base::binary);
  return digestpp::echo(512).absorb(file).hexdigest();
}



PYBIND11_MODULE(kerukuro_digestpp, m) {
    m.doc() = "pybind11 plugin";
    m.def("skein256",   &skein256,   "skein256");
    m.def("skein512",   &skein512,   "skein512");
    m.def("groestl256", &groestl256, "groestl256");
    m.def("groestl512", &groestl512, "groestl512");
    m.def("jh256",      &jh256,      "jh256");
    m.def("jh512",      &jh512,      "jh512");
    m.def("kupyna256",  &kupyna256,  "kupyna256");
    m.def("kupyna512",  &kupyna512,  "kupyna512");
    m.def("esch256",    &esch256,    "esch256");
    m.def("esch512",    &esch512,    "esch512");
    m.def("echo256",    &echo256,    "echo256");
    m.def("echo512",    &echo512,    "echo512");
}
