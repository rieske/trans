#!/bin/bash
mkdir build
cppcheck -v --enable=all --xml --xml-version=1 -I/usr/include/c++/4.8.1/ src/ 2> build/cppcheck-report.xml

