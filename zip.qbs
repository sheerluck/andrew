import qbs.base
CppApplication {
    name:  "zip"
    files: ["zip.cpp"]
    cpp.cxxFlags: ["-std=c++11"]
}
