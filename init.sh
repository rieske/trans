#!/bin/bash
rm -rf build
rm -f compile_commands.json
mkdir build
pushd .
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
popd
ln -s build/compile_commands.json compile_commands.json
