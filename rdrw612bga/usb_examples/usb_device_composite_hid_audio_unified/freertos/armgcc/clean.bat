RD /s /Q debug release flash_debug flash_release
set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_hid_audio_unified_freertos" (
  cd ../../../../../../build
  RD /s /Q dev_composite_hid_audio_unified_freertos
)
cd %batch_dir%
