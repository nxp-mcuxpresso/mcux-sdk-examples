# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(central_pxm)

set(MCUX_SDK_PROJECT_NAME central_pxm.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../app_config.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../wifi_bt_config.c"
"${ProjDirPath}/../wifi_bt_config.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../central_pxm.c"
"${ProjDirPath}/../central_pxm.h"
"${ProjDirPath}/../edgefast_bluetooth_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../bt_module_config.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs/lwipopts.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs/lwippools.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs/wifi_config.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_platform.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_platform.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/host_msd_fatfs.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/host_msd_fatfs.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs/ffconf.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs/usb_host_config.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/bt_db_gen.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_assert.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_config.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_debug_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_fops.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_os.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_serial.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_timer.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_a2dp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_assigned_numbers.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_at_parser_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_att_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_avctp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_avdtp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_avrcp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_avrcp_ca_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_bip_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_bnep_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_bottom_half.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_bpp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_common.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_ctn_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_dbase_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_device_queue.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_error.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_ftp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_gap_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_gatt_db_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_hci_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_hdp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_hfp_ag_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_hfp_unit_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_hid_device_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_hid_host_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_irobex_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_jpl_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_map_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_mcap_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_obex_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_opp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_pan_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_pbap_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_racp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_rfcomm_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_sap_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_sdp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_security.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_sm_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_smp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_snoop.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_spp_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_status.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_storage.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/BT_version.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/fsm_defines.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/fsm_engine.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/gatt_defines.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/hci_transport.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/l2cap.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include/write_task.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/vendor/vendor_specific_commands.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/vendor/vendor_specific_init.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/vendor/vendor_specific_init.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/att/att.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/avctp/avctp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/avdtp/avdtp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/bnep/bnep.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/dbase/database.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/hci_1.2/hci_h.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/mcap/mcap.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/obex/obex.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/rfcomm/rfcomm.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/sdp/sdp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/sm/sm.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/sm/sm_extern.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/sm/sm_internal.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_extern.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_fsm.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_fsm_engine.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_internal.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/aes_cmac/aes_cmac.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/at_parser/at_parser_hsp_hfp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/object_parser/object_parser_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/object_parser/tokenizer.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/racp/racp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/storage/storage.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_common_defines.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_encoder.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_parser.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_assert.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_config.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_config.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_debug.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_debug.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_debug_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_debug_internal.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_fops.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_fops.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_os.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_os.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_serial.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_serial.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_timer.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_timer.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos/EM_timer_internal.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_assert.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_config.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_debug.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_debug_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_fops.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_os.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_status_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_timer.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/btsnoop_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/btsnoop_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/ctn_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/ctn_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/ftp_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/ftp_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart_internal.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/ht_read_task_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/map_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/map_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/obex_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/obex_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/pbap_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/pbap_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_extern_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/littlefs_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/littlefs_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller_wifi_nxp.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller_wifi_nxp.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_assert.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_config.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_config.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug_internal.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_fops.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_fops.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_os.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_os.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_serial.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_serial.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_timer.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_timer.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_timer_internal.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/sbc/sbc_os.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/EM_platform.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/EM_platform.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/sbc/sbc_api.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config/BT_features.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config/BT_limits.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config/db_gen.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/aes_cmac_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/aes_cmac_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/BT_common_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/BT_common_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/device_queue_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/device_queue_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/sm_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/sm_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/sm_ssp_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/sm_ssp_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/smp_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/smp_pl.h"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/gatt_db_pl.c"
"${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension/gatt_db_pl.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_att.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_att_internal.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_conn.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_conn_internal.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_crypto.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_crypto_internal.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_gatt.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_gatt_internal.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_core.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_core.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_adv.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_adv.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_br.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_br.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_id.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_id.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_scan.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_scan.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ecc.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ecc.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_addr.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_buf.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_direction.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_direction_internal.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_ecc.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap_internal.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_monitor.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_monitor.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rpa.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rpa.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_settings.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_settings.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp_null.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ssp.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_uuid.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/addr.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/att.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/bluetooth.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/direction.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/buf.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/l2cap.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/conn.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/crypto.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/gap.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/gatt.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/hci.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/hci_err.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/hci_raw.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/hci_vs.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/iso.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/testing.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/uuid.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/sys/util.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/sys/printk.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/sys/byteorder.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/zephyr/types.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting/porting.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting/porting.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting/errno/errno.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting/work_queue.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/sys/work_queue.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/toolchain/common.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/toolchain/gcc.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/toolchain/iar.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/toolchain/armcc.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/toolchain.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting/buf.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/net/buf.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting/slist.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/sys/slist.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting/atomic_c.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/sys/atomic.h"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/services/pxr.c"
"${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include/bluetooth/services/pxr.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/controller
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/impl/ethermind
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/eOSAL
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/include
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/vendor
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/att
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/avctp
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/avdtp
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/bnep
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/common
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/dbase
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/hci_1.2
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/mcap
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/obex
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/rfcomm
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/sdp
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/sm
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/smp
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/protocols/write_task
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/aes_cmac
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/at_parser
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/btsnoop
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/object_parser
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/racp
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/security
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/status
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/storage
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/xml_parser
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/ethal
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux/sbc
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/osal/src/freertos
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/port/pal/mcux
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/utils/sbc
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config
    ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/export/extension
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/include
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/bluetooth
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/porting
    ${ProjDirPath}/../../../../../middleware/edgefast_bluetooth/source/services
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(device_startup)

include(driver_flexcomm_usart)

include(component_usart_adapter)

include(driver_power)

include(utility_assert)

include(utility_debug_console)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(middleware_sdmmc_sdio)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_freertos_MIMXRT685S_cm33)

include(driver_trng)

include(driver_flexspi)

include(middleware_usb_host_stack_MIMXRT685S_cm33)

include(component_osa)

include(middleware_usb_host_ip3516hs_MIMXRT685S_cm33)

include(middleware_usb_phy)

include(middleware_fatfs)

include(middleware_fatfs_usb)

include(component_mem_manager)

include(component_messaging)

include(component_software_rng_adapter)

include(middleware_freertos-kernel_heap_4)

include(middleware_freertos-kernel_MIMXRT685S_cm33)

include(middleware_wireless_ethermind_wifi_fwdnld)

include(middleware_edgefast_bluetooth_k32w061_controller)

include(driver_flash_config_evkmimxrt685)

include(driver_cache_cache64)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_lpc_gpio)

include(driver_reset)

include(component_flexspi_nor_flash_adapter)

include(component_flexspi_nor_flash_adapter_rt685evk)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_pca9420)

include(driver_flexcomm_i2c)

include(utilities_misc_utilities)

include(CMSIS_Include_core_cm)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(middleware_freertos-kernel_extension)

include(driver_usdhc)

include(middleware_usb_common_header)

include(middleware_usb_host_common_header)

include(middleware_usb_host_msd)

include(component_log_backend_debugconsole)

include(component_log)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33/gcc/libethermind_ble_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


