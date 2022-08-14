#! /bin/sh

cmake -S . -B build/ -G "MinGW Makefiles" -DSFML_BUILD_AUDIO=OFF -DSFML_BUILD_NETWORK=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Debug;
cd ./build/;
mingw32-make

