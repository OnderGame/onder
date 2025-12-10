#!/bin/sh

set -xe

cmake -S . -B build
make -C build
./build/Onder
