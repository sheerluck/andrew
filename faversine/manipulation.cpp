#include "manipulation.h"

#include <iostream>

#include <QFileInfo>
#include <QDir>
#include <QTextStream>

#include "bits.h"
#include "avg.h"
#include "range.h"
#include "visualisation.h"
#include "functional.h"
#include <QImage>   // but WHY? o_0

namespace manipulation {

model::Grid
makeGrid(const model::VTLF& temperatures)
{
    auto result = model::Grid{};
    for (const qint16 x : range(-185, 185))
    for (const qint16 y : range( -95,  95))
    {
        if (0 == y) std::cout << x << '\n';
        const auto key = to32(x, y);
        const auto lat = static_cast<float>(y);
        const auto lon = static_cast<float>(x);
        const auto Lok = model::Location{lat, lon};  // Y!!! X!!! NOT X,Y
        result[key]    = visualisation::predictTemperature(temperatures, Lok);
    }
    return result;
}

void
save2text(const int year,
          const model::Grid& grid)
{
    const auto root = "/home/sheerluck/tmp/scala/resources/";
    const auto name = QString::number(year) + ".grid";
    const auto path = QString(root) + name;
    if (QFileInfo::exists(path))
    {
        std::cout << QString("%1 exists!!!\n").arg(name).toStdString();
    }
    else
    {
        auto output = QFile{path};
        if (output.open(QFile::WriteOnly | QFile::Truncate))
        {
            auto out = QTextStream{&output};
            auto acc = 0;
            for (const auto& [key, val] : grid )
            {
                const auto& [a, b] = from(key);
                union cast {
                  float f;
                  unsigned int ui;
                } c;
                c.f = val;
                const auto  line   = QString("%1,%2,%3\n").arg(b).arg(a).arg(c.ui);
                out << line;  // "lat,lon,t"
                acc += 1;
                if (acc > 10000)
                {
                    acc = 0;
                    std::cout << "*";
                }
            }
        }
    }
}

model::Grid
loadFromText(const int       year)
{
    const auto root = "/home/sheerluck/tmp/scala/resources/";
    const auto name = QString::number(year) + ".grid";
    const auto path = QString(root) + name;
    if (QFileInfo::exists(path))
    {
        const auto f = [](const auto a)
        {
            bool ok;
            unsigned q = a.toLong(&ok);
            return ok ? Opt<unsigned>{q} : Opt<unsigned>{};
        };

        const auto g = [](const auto a)
        {
            bool ok;
            int q = a.toInt(&ok);
            return ok ? Opt<qint16>{q} : Opt<qint16>{};
        };

        auto getLines = [root](const auto fn)
        {
            auto result = std::vector<QString>{};
            auto input  = QFile{QString{root} + fn};
            if (input.open(QIODevice::ReadOnly))
            {
                auto in = QTextStream{&input};
                while (!in.atEnd()) result.emplace_back(in.readLine());
            }
            return result;
        };

        auto parse = [f,g](const QString line)
        {
            const auto  a    = line.splitRef(",");
            const auto& [lat, lon, t] = std::make_tuple(a[0], a[1], a[2]);
            return std::make_tuple(g(lat), g(lon), f(t));
        };

        const auto Lines = getLines(name);
              auto grid  = model::Grid{};

        for (const auto& [lat, lon, t] : fmap(parse, Lines))
        {
            const auto key = to32(lon.value(), lat.value());
            union cast {
              float f;
              unsigned int ui;
            } c;
            c.ui = t.value();
            grid[key] = c.f;
        }
        return grid;
    }
    return {};
}


model::Grid
average(const model::wtf& gridvec)
{
    auto result = model::Grid{};
    for (const qint16 x : range(-185, 185))
    for (const qint16 y : range( -95,  95))
    {
        if (0 == y) std::cout << x << '\n';
        const auto key = to32(x, y);
        const auto ff   = [key](auto f) { return f[key]; };
        const auto allt = fmap(ff, gridvec);
        result[key]     = avg(allt);
    }
    return result;
}


model::Grid
deviation(const model::VTLF& temperatures,
          const model::Grid& normals)
{
    std::cout << temperatures.size();
    return normals;
}


}
