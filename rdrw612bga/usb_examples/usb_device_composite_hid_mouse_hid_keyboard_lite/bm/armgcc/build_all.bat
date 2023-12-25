set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" (
  cd ../../../../../../build
  RD /s /Q dev_composite_hid_mouse_hid_keyboard_lite_bm
)
cd %batch_dir%
md "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -S . -B "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" -DCMAKE_BUILD_TYPE=flash_debug  "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cd %batch_dir%/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm
mingw32-make -j

set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" (
  cd ../../../../../../build
  RD /s /Q dev_composite_hid_mouse_hid_keyboard_lite_bm
)
cd %batch_dir%
md "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -S . -B "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" -DCMAKE_BUILD_TYPE=flash_release  "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cd %batch_dir%/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm
mingw32-make -j

set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" (
  cd ../../../../../../build
  RD /s /Q dev_composite_hid_mouse_hid_keyboard_lite_bm
)
cd %batch_dir%
md "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -S . -B "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" -DCMAKE_BUILD_TYPE=debug  "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cd %batch_dir%/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm
mingw32-make -j

set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" (
  cd ../../../../../../build
  RD /s /Q dev_composite_hid_mouse_hid_keyboard_lite_bm
)
cd %batch_dir%
md "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cmake -DCMAKE_TOOLCHAIN_FILE="../../../../../../core/tools/cmake_toolchain_files/armgcc.cmake" -G "MinGW Makefiles" -S . -B "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" -DCMAKE_BUILD_TYPE=release  "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm"
cd %batch_dir%/../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm
mingw32-make -j

IF "%1" == "" ( pause )
