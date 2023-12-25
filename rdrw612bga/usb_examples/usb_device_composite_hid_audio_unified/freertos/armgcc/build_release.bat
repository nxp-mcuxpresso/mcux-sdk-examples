set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_hid_audio_unified_freertos" (
  cd ../../../../../../build
  RD /s /Q dev_composite_hid_audio_unified_freertos
)
cd %batch_dir%
md "../../../../../../build/dev_composite_hid_audio_unified_freertos"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -S . -B "../../../../../../build/dev_composite_hid_audio_unified_freertos" -DCMAKE_BUILD_TYPE=release "../../../../../../build/dev_composite_hid_audio_unified_freertos"
cd %batch_dir%/../../../../../../build/dev_composite_hid_audio_unified_freertos
mingw32-make -j 2> build_log.txt 
