# Makefile Generator

It generates a makefile for c++ programes starting from a spefication, e.g.

```
    global { cxxflags {-Wall} libflags {} } 

    exec { name {exec_name} srcs {file.cpp, file2.cpp, file3.cpp} }
    exec { name {exec_name} srcs {file4.cpp, file5.cpp, file2.cpp} }
    exec { name {exec_name} srcs {file.cpp} lib {-lrt} }
'''

