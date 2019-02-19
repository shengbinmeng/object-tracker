#!/bin/bash

# Retrieve OpenCV code.
git submodule update --init

# Build OpenCV.
cd opencv
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=./local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_LAPACK:bool=OFF -DBUILD_SHARED_LIBS:bool=OFF ..
make -j 8
make install
cd ../..

# Set environment variable for pkg-config.
export PKG_CONFIG_PATH=$PWD/opencv/build/local/lib/pkgconfig
