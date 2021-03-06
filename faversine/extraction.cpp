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

#include "functional.h"
#include "avg.h"

namespace extraction {

model::VTQLF
locateTemperatures(const int year,
                   const SV stationsFileName,
                   const SV temperaturesFileName)
{
    const auto f = [](const auto a)
    {
        bool ok;
        float q = a.toFloat(&ok);
        return ok ? Opt<float>{q} : Opt<float>{};
    };

    const auto g = [](const auto a)
    {
        bool ok;
        int q = a.toInt(&ok);
        return ok ? Opt<float>{q} : Opt<float>{};
    };

    auto getLines = [](const SV name)
    {
        const auto root   = "/home/sheerluck/tmp/scala/resources";
              auto result = std::vector<QString>{};
              auto input  = QFile{QString{root} + name.data()};
        if (input.open(QIODevice::ReadOnly))
        {
            auto in = QTextStream{&input};
            while (!in.atEnd()) result.emplace_back(in.readLine());
        }
        return result;
    };

    auto parseSt = [f](const QString line)
    {
        const auto  a    = line.splitRef(",");
        const auto& [stn, wban, Lat, Lon] = std::make_tuple(a[0], a[1], a[2], a[3]);
        const auto  STN  = ( stn.isEmpty() ? QString{"000000"} :  stn.toString());
        const auto  WBAN = (wban.isEmpty() ? QString{ "00000"} : wban.toString());
        const auto  code = QString("%1%2").arg(STN).arg(WBAN);
        return std::make_tuple(code.toStdString(), f(Lat), f(Lon));
    };

    const auto stationsLines = getLines(stationsFileName);
          auto statimap = model::MSL{};

    for (const auto& [key, opa, opb] : fmap(parseSt, stationsLines))
    {
      if (opa)
        if (opb)
          statimap[key] = model::Location{opa.value(), opb.value()};
    }

    auto parseT = [f,g](const QString line)
    {
        const auto  a    = line.splitRef(",");
        const auto& [stn, wban, MM, DD, T] = std::make_tuple(a[0], a[1], a[2], a[3], a[4]);
        const auto  STN  = ( stn.isEmpty() ? QString{"000000"} :  stn.toString());
        const auto  WBAN = (wban.isEmpty() ? QString{ "00000"} : wban.toString());
        const auto  code = QString("%1%2").arg(STN).arg(WBAN);
        return std::make_tuple(code.toStdString(), g(MM), g(DD), f(T));
    };

    const auto getDate = [](int y, int m, int d) { return QDate{y, m, d}; };
    const auto k =   5.f / 9.f;
    const auto y = 160.f / 9.f;
    const auto f2c  = [k,y](float x) { return k*x - y; };

    const auto temLines = getLines(temperaturesFileName);
          auto temper   = model::VTQLF{};

    for (const auto& [key, opm, opd, opt] : fmap(parseT, temLines))
    {
      if (opm)
        if (opd)
          if (opt)
            temper.emplace_back(
                     getDate(
                        year,
                        opm.value(),     // guess what?
                        opd.value()),    // it's not Scala
                     statimap[key],      // no for-yield for you
                     f2c(opt.value())
                   );
    }

    return temper;
}

model::VTLF
locationYearlyAverageRecords(const model::VTQLF& records)
{
    // groupBy
    auto groups = model::MLVFh{};
    auto vt     = std::vector<float>{3.1415926};
    for (const auto& [date, Loc, t] : records)
    {
        vt[0] = t;
        auto [it, ok] = groups.try_emplace(Loc, vt);
        if (!ok)
        {
            // sad thing is I can't just it->emplace_back(t);
            auto& [key, val] = *it;
            val.emplace_back(t);
            vt[0] = key.lat; // gcc: C++17 can't [[maybe_unused]] with Structured Bindings
        }
        date.isValid();      // gcc: C++17 can't [[maybe_unused]] with Structured Bindings
    }

    auto result = model::VTLF{};
    for (const auto& [key, val] : groups)
    {
        result.emplace_back(key, avg(val));
    }
    return result;
}



}

