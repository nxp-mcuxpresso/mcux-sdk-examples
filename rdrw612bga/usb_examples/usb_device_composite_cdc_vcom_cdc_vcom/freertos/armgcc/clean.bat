RD /s /Q debug release flash_debug flash_release
set batch_dir=%~dp0
if exist "../../../../../../build/dev_composite_cdc_vcom_cdc_vcom_freertos" (
  cd ../../../../../../build
  RD /s /Q dev_composite_cdc_vcom_cdc_vcom_freertos
)
cd %batch_dir%
