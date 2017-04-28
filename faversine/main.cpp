#include <iostream>
#include <tuple>

#include <QString>

#include "range.h"
#include "data.h"
#include "interaction.h"
#include "model.h"

int main()
{
    /*  test Qt5/C++17
    auto tuple  = std::make_tuple(1, 'a', 2.3);
    auto [a, b, c] = tuple;
    QString all = QString("{%1},{%2},{%3}").arg(a).arg(b).arg(c);
    std::cout << "Hello\n" << all.toStdString() << '\n';
    */

    for (const auto year : range(1975, 2016))
    {
        std::cout << year << '\n';
        auto data   = data::data(year);
        auto single = model::VTID{data};
        interaction::generateTiles(single);
    }

}
