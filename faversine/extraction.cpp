#include "extraction.h"

#include <vector>
#include <tuple>
#include <iostream>

#include <QDateTime>

#include "model.h"

namespace extraction {

model::VDTMLT
locateTemperatures(int year,
                   std::string stationsFileName,
                   std::string temperaturesFileName)
{
    std::cout << "kill me plz " << stationsFileName
              << " oh plz"      << temperaturesFileName
              << "oh noooooo!!!111\n";
    auto tempLoc = model::Location{0.2, 0.4};
    auto now     = QDateTime::currentDateTime();
    auto res = std::make_tuple(now,
                               tempLoc,
                               11.11 - year);
    return {res};
}



}

