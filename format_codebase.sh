#!/bin/sh
cpp_files=$(find src/ -name "*cpp")
hpp_files=$(find src/ -name "*hpp")
clang-format --style=file -i $cpp_files $hpp_files
