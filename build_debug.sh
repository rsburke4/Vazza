#!/bin/bash
cmake cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cd build
make
cd ..
