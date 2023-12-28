#!/bin/sh
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/xclang.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=debug  .
make -j
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/xclang.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=release  .
make -j
