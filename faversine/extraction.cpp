#include "extraction.h"

#include <vector>
//#include <tuple>
#include <iostream>

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QString>

#include "model.h"

namespace extraction {

model::VDTMLT
locateTemperatures(const int year,
                   const model::SV stationsFileName,
                   const model::SV temperaturesFileName)
{
    auto getLines = [](const model::SV name)
    {
        auto result = std::vector<QString>{};
        auto input  = QFile{QString(name.data())};
        if (input.open(QIODevice::ReadOnly))
        {
            auto in = QTextStream{&input};
            while (!in.atEnd()) result.emplace_back(in.readLine());
        }
        return result;
    };

    auto stationsLines = getLines(stationsFileName);

    std::cout << "kill me plz " << stationsFileName
              << " oh plz"      << temperaturesFileName
              << "oh noooooo!!!111\n";
    auto tempLoc = model::Location{0.2, 0.4};
    auto now     = QDateTime::currentDateTime();
    return {{now, tempLoc, 11.11 - year}};

}

model::VMLT
locationYearlyAverageRecords(const model::VDTMLT)
{
    auto x = 6;
}



}

