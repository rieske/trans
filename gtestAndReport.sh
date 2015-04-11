#!/bin/sh
./gtest --gtest_output=xml:build/gtest-report.xml
cat test-suite.log
