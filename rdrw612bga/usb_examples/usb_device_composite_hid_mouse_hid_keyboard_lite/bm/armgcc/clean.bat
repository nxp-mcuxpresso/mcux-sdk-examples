RD /s /Q debug release flash_debug flash_release
set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_hid_mouse_hid_keyboard_lite_bm" (
  cd ../../../../../../build
  RD /s /Q dev_composite_hid_mouse_hid_keyboard_lite_bm
)
cd %batch_dir%
