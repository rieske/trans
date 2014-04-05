#!/bin/bash
mkdir build
cppcheck -v --enable=all --xml --xml-version=1 -j 8 -Isrc src 2> build/cppcheck-report.xml
valgrind --xml=yes --xml-file=build/valgrind-report.xml ./trans test_prog1

