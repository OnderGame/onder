#!/bin/sh

set -xe

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
make -C build
exec ./build/Onder
