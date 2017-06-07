#include <iostream>

#include "range.h"
#include "data.h"
#include "model.h"
#include "interaction.h"
#include "manipulation.h"

int main()
{

    /*for (const auto year : range(1975, 2016))
    {
        std::cout << "\nyear=" << year << '\n';
        auto data   = data::data(year);
        auto single = model::VTID{data};
        interaction::generateTiles(single);
    }*/

    /*
    for (const auto year : range(1975, 2016))
    {
        const auto& [y, lt] = data::data(year);
        std::cout << "\nyear=" << y << '\n';
        const auto grid = manipulation::makeGrid(lt);
        manipulation::save2text(y, grid);
    }
    */

    //  0,0,1090903441 -> grid[(0,0)]  =  8.36659
    // 10,0,1103560031 -> grid[(10,0)] = 24.8737

    /*
    // normal grid
    auto wtf = model::wtf{};
    for (const auto year : range(1975, 1990))
    {
        std::cout << "\nyear=" << year << '\n';
        wtf.emplace_back(manipulation::loadFromText(year));
    }
    const auto grid = manipulation::average(wtf);
    manipulation::save2text(9000, grid);
    */

    /*
    // deviation
    const auto norm = manipulation::loadFromText(9000);
    for (const auto year : range(1990, 2016))
    {
        std::cout << "\nyear=" << year << '\n';
        const auto grid = manipulation::loadFromText(year);
        const auto dev  = manipulation::deviation(grid, norm);
        manipulation::save2text(5000 + year, dev);
    }*/


}
