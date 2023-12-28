cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/xclang.cmake" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=release  .
mingw32-make -j
IF "%1" == "" ( pause ) 
