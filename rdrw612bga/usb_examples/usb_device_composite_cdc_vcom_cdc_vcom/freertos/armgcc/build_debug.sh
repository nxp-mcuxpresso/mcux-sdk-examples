#!/bin/sh
script_dir=$(dirname "$0")
if [ -d "$script_dir/../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos" ];then rm -rf "$script_dir/../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos"; fi
mkdir -p "$script_dir/../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -S $script_dir -B "$script_dir/../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos" -DCMAKE_BUILD_TYPE=debug  "$script_dir/../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos"
cd $script_dir/../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos
make -j 2>&1 | tee build_log.txt
