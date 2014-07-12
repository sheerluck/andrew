#include <QCoreApplication>

#include "serv.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Plugin p;
    auto x1 = ConfigFile(23, 71, 34.2, 0.0101);
    p.init(x1);
    p.work();

    auto x2 = ConfigFile(11, 50, -2.2, 111.11);
    p.reload(x2);
    p.work();

    auto x3 = ConfigFile(77, 77, 3.141592, -1.01);
    p.reload(x3);
    p.work();

    p.done();
}
