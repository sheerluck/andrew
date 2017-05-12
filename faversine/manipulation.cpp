#include "manipulation.h"

#include <iostream>

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

#include "bits.h"
#include "range.h"
#include "visualisation.h"

namespace manipulation {

model::Grid
makeGrid(const model::VTLF& temperatures)
{
    auto result = model::Grid{};
    for (const qint16 x : range(-185, 185))
    for (const qint16 y : range( -95,  95))
    {
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

            //out << "Result: " << qSetFieldWidth(10) << left << 3.14 << 2.7;
            // writes "Result: 3.14      2.7       "
            for (const auto& [key, val] : grid )
            {

            }
        }
    }
}


}
