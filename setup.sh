#!/bin/sh
rm -rf build
export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build
