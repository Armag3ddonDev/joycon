#!/bin/bash

pacman -Syuu --noconfirm
pacman -S --needed --noconfirm $MINGW_PACKAGE_PREFIX-{cmake,ninja,qtcreator,toolchain,hidapi}


PROJECTPATH=$(cygpath ${APPVEYOR_BUILD_FOLDER})

mkdir build
cd build
cmake -G Ninja $PROJECTPATH

ninja -j2 #build

ninja test #run tests
