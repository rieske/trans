#!/bin/bash
mkdir build
pushd .
cd build
cmake ..
make
popd
./build/trans fibb.src
./build/trans -gresources/configuration/grammar.bnf -lp fibb.src
cp logs/parsing_table resources/configuration/parsing_table
./build/trans fibb.src
