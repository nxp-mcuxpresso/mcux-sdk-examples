#!/bin/sh
script_dir=$(dirname "$0")
if [ -d "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" ];then rm -rf "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"; fi
mkdir -p "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -S $script_dir -B "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" -DCMAKE_BUILD_TYPE=release  "$script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cd $script_dir/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm
make -j 2>&1 | tee build_log.txt
