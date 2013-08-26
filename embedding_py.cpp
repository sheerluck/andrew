/*

    n00b's first steps in embedding python:

    www.compsci.wm.edu/sciclone/documentation/software/compilers/Python-2.7.2/pdf/extending.pdf
    $ python3.3-config --cflags
    -I/usr/include/python3.3 -I/usr/include/python3.3 -Wno-unused-result -DNDEBUG -O2 -pipe -fwrapv -O2 -pipe -fwrapv
    $ python3.3-config --ldflags
    -lpthread -ldl -lutil -lm -lpython3.3 -Xlinker -export-dynamic
    $ g++ --std=c++11 embedding_py.cpp -o embedding_py `python3.3-config --cflags` `python3.3-config --ldflags`
    $ ./embedding_py
    Today is Mon Aug 26 18:37:15 2013

*/

#include <Python.h>
int main()
{
    Py_Initialize();
    PyRun_SimpleString("from time import time, ctime\n"
                       "print('Today is', ctime(time()))\n");
    Py_Finalize();
    return 0;
}

