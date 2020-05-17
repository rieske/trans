#!/bin/bash
rm -rf build
mkdir build
pushd .
cd build
cmake ..
popd
ln -s build/compile_commands.json compile_commands.json
