#include "extraction.h"

#include <vector>
#include <tuple>
#include <iostream>

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QVector>
#include <QStringRef>

#include "model.h"
#include "functional.h"

namespace extraction {

model::VDTMLT
locateTemperatures(const int year,
                   const SV stationsFileName,
                   const SV temperaturesFileName)
{
    auto getLines = [](const SV name)
    {
        const auto root = "/home/sheerluck/tmp/scala/resources";
        auto result = std::vector<QString>{};
        auto input  = QFile{QString{root} + name.data()};
        if (input.open(QIODevice::ReadOnly))
        {
            auto in = QTextStream{&input};
            while (!in.atEnd()) result.emplace_back(in.readLine());
        }
        return result;
    };

    auto parseSt = [](QString line)
    {
        const auto f= [](auto a) {
          // Try(a.toDouble).toOption.flatMap(Option.apply)
          bool ok;
          float q = a.toFloat(&ok);
          if (ok)
              return Opt<float>{q};
          else
              return Opt<float>{};
        };

        const auto a = line.splitRef(",");
        const auto [stn, wban, Lat, Lon] = std::make_tuple(a[0], a[1], a[2], a[3]);
        const auto STN  = ( stn.isEmpty() ? QString{"000000"} :  stn.toString());
        const auto WBAN = (wban.isEmpty() ? QString{ "00000"} : wban.toString());
        const auto code = QString("%1%2").arg(STN).arg(WBAN);
        return std::make_tuple(code, f(Lat), f(Lon));
    };

    const auto stationsLines = getLines(stationsFileName);
    //const auto stations =

    for (const auto [key, opa, opb] : fmap(parseSt, stationsLines))
    {
        std::cout << key.toStdString() << '\n';
    }

    auto xxx = 15.7 / (2 - 2);
    std::cout << xxx << "!!!\n";

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
    return {};
}



}

