#include "data.h"

#include <iostream>

#include <QString>
#include <QFileInfo>
#include <QDir>

#include "extraction.h"
#include "interaction.h"

namespace data {

model::TID
data(int year)
{
    auto test = model::VMLT{2};
    return std::make_tuple(year, test);
} // data


void
gen(int year, int zoom,
    int x,    int y,
    model::VMLT data)
{
    const auto p = 255;
    const auto table = model::VFC{
        { 60.0, model::Color{p,p,p}},
        { 32.0, model::Color{p,0,0}},
        { 12.0, model::Color{p,p,0}},
        {  0.0, model::Color{0,p,p}},
        {-15.0, model::Color{0,0,p}},
        {-27.0, model::Color{p,0,p}},
        {-50.0, model::Color{33,0,107}},
        {-60.0, model::Color{0,0,0}},
    };

    const auto root = "/home/sheerluck/tmp/scala/";
    const auto qdot = QDir{root};
    std::cout << QString("generateImage(%1, %2, %3, %4, %5)")
                        .arg(year).arg(zoom).arg(x).arg(y).arg(data.size())
                        .toStdString() << '\n';
    const auto name = QString("target/temperatures/%1/%2/%3-%4.png")
                             .arg(year).arg(zoom).arg(x).arg(y);
    const auto path = QString(root) + name;
    if (QFileInfo::exists(path))
    {
        std::cout << QString("%1 exists!!!").arg(name).toStdString() << '\n';
    }
    else
    {
        // sweet baby jesus christ! parent directory oneliner! OMFG!
        auto parentDir = QString(QFileInfo(path).dir().absolutePath());
        qdot.mkpath(parentDir);
        std::cout << QString("before %1")
                     .arg(QDateTime::currentDateTime().toString())
                     .toStdString() << '\n';
        auto img  = interaction::tile(data, table, zoom, x, y);
        std::cout << QString("after %1")
                     .arg(QDateTime::currentDateTime().toString())
                     .toStdString() << '\n';

        std::cout << (img.save(path) ? "saved\n" : "failed");
    }
} // gen

}
