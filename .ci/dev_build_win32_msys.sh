#!/usr/bin/env bash

#
# Building on MSYS2
#

export MSYSTEM=MINGW64
export PATH=/mingw64/bin:$PATH

CONFIG=Release
cmake .. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=$CONFIG
cmake --build . --config $CONFIG
ctest -C $CONFIG --output-on-failure
