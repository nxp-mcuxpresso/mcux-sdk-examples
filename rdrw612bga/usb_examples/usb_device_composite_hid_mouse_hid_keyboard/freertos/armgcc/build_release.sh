#!/bin/sh
script_dir=$(dirname "$0")
if [ -d "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_freertos" ];then rm -rf "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_freertos"; fi
mkdir -p "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_freertos"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -S $script_dir -B "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_freertos" -DCMAKE_BUILD_TYPE=release  "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_freertos"
cd $script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_freertos
make -j 2>&1 | tee build_log.txt
