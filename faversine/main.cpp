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
    
    for (const auto year : range(1975, 2016))
    {
        const auto& [y, lt] = data::data(year);
        std::cout << "\nyear=" << y << '\n';
        const auto grid = manipulation::makeGrid(lt);
        manipulation::save2text(y, grid);
    }
    
}
