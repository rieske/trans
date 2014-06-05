#!/bin/bash
make check
./trans test_prog1
./trans -ggrammar.bnf -lp test_prog1
cp logs/parsing_table parsing_table
./trans test_prog1
