cython --cplus parallel_sort.pyx

g++ -std=c++2a -march=native -mtune=native -O3   \
-maes -mavx -mavx2 -mf16c -mfma -mmmx -mpclmul   \
-mpopcnt -msse -msse2 -msse3 -msse4.1 -msse4.2   \
-mssse3 -pedantic -Wextra -Wshadow               \
-Wsign-compare -Wwrite-strings -Wpointer-arith   \
-Winit-self -Wconversion -Wno-sign-conversion    \
-Wsuggest-override -Wnon-virtual-dtor            \
-Wdelete-non-virtual-dtor -fgraphite-identity    \
-floop-nest-optimize -fdevirtualize-at-ltrans    \
-fipa-pta -fno-semantic-interposition -flto=9    \
-fuse-linker-plugin -pipe -falign-functions=32   \
-fpic -c parallel_sort.cpp -o parallel_sort.o \
-fopenmp `python-config --includes`         \
-I`python -c "import numpy as np; print(np.get_include())"`

g++ -std=c++2a -march=native -mtune=native -O3   \
-maes -mavx -mavx2 -mf16c -mfma -mmmx -mpclmul   \
-mpopcnt -msse -msse2 -msse3 -msse4.1 -msse4.2   \
-mssse3 -pedantic -Wextra -Wshadow               \
-Wsign-compare -Wwrite-strings -Wpointer-arith   \
-Winit-self -Wconversion -Wno-sign-conversion    \
-Wsuggest-override -Wnon-virtual-dtor            \
-Wdelete-non-virtual-dtor -fgraphite-identity    \
-floop-nest-optimize -fdevirtualize-at-ltrans    \
-fipa-pta -fno-semantic-interposition -flto=9    \
-fuse-linker-plugin -pipe -falign-functions=32   \
-shared -o parallel_sort.so parallel_sort.o `python-config --libs` -lgomp

rm -f parallel_sort.cpp parallel_sort.o
