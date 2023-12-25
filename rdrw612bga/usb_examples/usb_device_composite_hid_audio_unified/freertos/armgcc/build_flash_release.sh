#!/bin/sh
script_dir=$(dirname "$0")
if [ -d "$script_dir/../../../../../../build/dev_composite_hid_audio_unified_freertos" ];then rm -rf "$script_dir/../../../../../../build/dev_composite_hid_audio_unified_freertos"; fi
mkdir -p "$script_dir/../../../../../../build/dev_composite_hid_audio_unified_freertos"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -S $script_dir -B "$script_dir/../../../../../../build/dev_composite_hid_audio_unified_freertos" -DCMAKE_BUILD_TYPE=flash_release  "$script_dir/../../../../../../build/dev_composite_hid_audio_unified_freertos"
cd $script_dir/../../../../../../build/dev_composite_hid_audio_unified_freertos
make -j 2>&1 | tee build_log.txt
