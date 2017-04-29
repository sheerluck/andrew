#include <iostream>

#include "range.h"
#include "data.h"
#include "model.h"
#include "interaction.h"

int main()
{
    for (const auto year : range(1975, 2016))
    {
        std::cout << year << '\n';
        auto data   = data::data(year);
        //auto single = model::VTID{data};
        //interaction::generateTiles(single);
    }
}
