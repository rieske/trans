#!/bin/bash
make check
./trans fibb.src
./trans -gresources/configuration/grammar.bnf -lp fibb.src
cp logs/parsing_table resources/configuration/parsing_table
./trans fibb.src
