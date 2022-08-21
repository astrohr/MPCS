#! /bin/sh

cmake -S . -B build/ -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release;
cd ./build/;
mingw32-make
