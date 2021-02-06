#!/bin/bash
rm -rf build
rm -f compile_commands.json
mkdir build
pushd .
cd build
cmake ..
popd
ln -s build/compile_commands.json compile_commands.json
