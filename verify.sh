#!/bin/bash
make check
./trans test_prog1
./trans -gresources/configuration/grammar.bnf -lp test_prog1
cp logs/parsing_table resources/configuration/parsing_table
./trans test_prog1
