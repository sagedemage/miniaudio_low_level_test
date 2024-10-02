#!/bin/sh
cpp_files=$(find ../src/ -name "*cpp")
hpp_files=$(find ../src/ -name "*hpp")
include_hpp_files=$(find ../include/ -name "*hpp")
clang-tidy --format-style=file $cpp_files $hpp_files $include_hpp_files
