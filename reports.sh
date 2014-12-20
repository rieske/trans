#!/bin/bash
mkdir -p build

gcovr -x -r . > build/gcovr-report.xml

cppcheck -v --enable=all --xml --xml-version=1 -j 8 -Isrc src 2> build/cppcheck-report.xml
sed -i.bak s/\ file=\"src\\//\ file=\".\\/src\\//g build/cppcheck-report.xml

#find src -regex ".*\.cpp\|.*\.h" | vera++ - -showrules -nodup |& scripts/vera++Report2checkstyleReport.perl > build/vera-report.xml

rats -w 3 --xml src > build/rats-report.xml

valgrind --xml=yes --xml-file=build/valgrind-report.xml ./trans test_prog1
