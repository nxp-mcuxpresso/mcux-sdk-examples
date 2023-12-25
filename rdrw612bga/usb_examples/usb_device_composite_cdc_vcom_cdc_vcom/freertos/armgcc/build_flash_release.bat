set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos" (
  cd ../../../../../../build
  RD /s /Q dev_composite_cdc_vcom_cdc_vcom_freertos
)
cd %batch_dir%
md "../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -S . -B "../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos" -DCMAKE_BUILD_TYPE=flash_release "../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos"
cd %batch_dir%/../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos
mingw32-make -j 2> build_log.txt 
