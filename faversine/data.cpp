#include "data.h"

#include <iostream>

#include <QString>
#include <QFileInfo>
#include <QDir>

#include "extraction.h"
#include "interaction.h"

namespace data {

model::TID
data(const int year)
{
    const auto name = QString("/%1.csv").arg(year).toStdString();
    const auto dlt  = extraction::locateTemperatures(year, "/stations.csv", name);
    std::cout      << QString("dlt.size=%1\n").arg(dlt.size()).toStdString();
    const auto lt   = extraction::locationYearlyAverageRecords(dlt);
    std::cout      << QString("lt.size=%1\n").arg(lt.size()).toStdString();
    return {year, lt};
}


void
gen(const int year, const int zoom,
    const int x,    const int y,
    const model::VTLF& data)
{
    const auto root = "/home/sheerluck/tmp/scala/";
    const auto qdot = QDir{root};
    std::cout << QString("generateImage(%1, %2, %3, %4, %5)\n")
                        .arg(year).arg(zoom).arg(x).arg(y).arg(data.size())
                        .toStdString();
    const auto name = QString("target/temperatures/%1/%2/%3-%4.png")
                             .arg(year).arg(zoom).arg(x).arg(y);
    const auto path = QString(root) + name;
    if (QFileInfo::exists(path))
    {
        std::cout << QString("%1 exists!!!\n").arg(name).toStdString();
    }
    else
    {
        // sweet baby jesus christ! parent directory oneliner! OMFG!
        auto parentDir = QString(QFileInfo(path).dir().absolutePath());
        qdot.mkpath(parentDir);
        std::cout << QString("before %1\n")
                     .arg(QDateTime::currentDateTime().toString())
                     .toStdString();
        auto img  = interaction::tile(data, /*table,*/ zoom, x, y);
        std::cout << QString("after  %1\n")
                     .arg(QDateTime::currentDateTime().toString())
                     .toStdString();

        std::cout << (img.save(path) ? "saved\n\n" : "failed\n\n");
    }
} // gen

}
