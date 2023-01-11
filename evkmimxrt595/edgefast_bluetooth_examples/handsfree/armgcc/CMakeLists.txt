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

if (DEFINED ENV{SdkRootDirPath})
    SET(SdkRootDirPath "$ENV{SdkRootDirPath}")
else()
    SET(SdkRootDirPath "${ProjDirPath}/../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(handsfree)

set(MCUX_BUILD_TYPES flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME handsfree.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../app_config.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../wifi_bt_config.c"
"${ProjDirPath}/../wifi_bt_config.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs/mbedtls_config_client.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../app_handsfree.c"
"${ProjDirPath}/../app_handsfree.h"
"${ProjDirPath}/../app_shell.c"
"${ProjDirPath}/../app_shell.h"
"${ProjDirPath}/../sco_audio_pl_ext.c"
"${ProjDirPath}/../ringtone.h"
"${ProjDirPath}/../edgefast_bluetooth_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../bt_module_config.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs/lwipopts.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs/lwippools.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs/wifi_config.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_platform.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_platform.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/host_msd_fatfs.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/host_msd_fatfs.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs/ffconf.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs/usb_host_config.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/bt_db_gen.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_assert.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_config.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_debug_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_fops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_serial.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_timer.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_a2dp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_assigned_numbers.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_at_parser_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_att_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_avctp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_avdtp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_avrcp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_avrcp_ca_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_bip_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_bnep_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_bottom_half.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_bpp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_common.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_ctn_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_dbase_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_device_queue.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_error.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_ftp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_gap_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_gatt_db_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_hci_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_hdp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_hfp_ag_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_hfp_unit_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_hid_device_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_hid_host_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_irobex_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_jpl_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_map_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_mcap_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_obex_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_opp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_pan_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_pbap_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_racp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_rfcomm_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_sap_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_sdp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_security.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_sm_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_smp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_snoop.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_spp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_status.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_storage.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_version.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/fsm_defines.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/fsm_engine.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/gatt_defines.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/hci_transport.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/l2cap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/write_task.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/vendor/vendor_specific_commands.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/vendor/vendor_specific_init.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/vendor/vendor_specific_init.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/att/att.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/avctp/avctp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/avdtp/avdtp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/bnep/bnep.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/dbase/database.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/hci_1.2/hci_h.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/mcap/mcap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/obex/obex.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/rfcomm/rfcomm.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/sdp/sdp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/sm/sm.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/sm/sm_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/sm/sm_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_fsm.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_fsm_engine.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/smp/smp_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/aes_cmac/aes_cmac.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/at_parser/at_parser_hsp_hfp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/object_parser/object_parser_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/object_parser/tokenizer.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/racp/racp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/storage/storage.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_common_defines.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_encoder.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_parser.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_assert.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_config.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_config.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_debug.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_debug.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_debug_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_debug_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_fops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_fops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_os.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_serial.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_serial.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_timer.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_timer.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos/EM_timer_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_assert.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_config.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_debug.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_debug_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_fops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_status_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_timer.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/btsnoop_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/btsnoop_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ctn_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ctn_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ftp_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ftp_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ht_read_task_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/map_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/map_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/obex_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/obex_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/pbap_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/pbap_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_extern_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/littlefs_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/littlefs_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_assert.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_config.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_config.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_fops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_fops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_os.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_serial.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_serial.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_timer.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_timer.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_timer_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/sbc/sbc_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/EM_platform.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/EM_platform.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/sbc/sbc_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config/BT_features.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config/BT_limits.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config/db_gen.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/aes_cmac_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/aes_cmac_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/BT_common_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/BT_common_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/device_queue_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/device_queue_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/sm_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/sm_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/sm_ssp_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/sm_ssp_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/smp_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/smp_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/gatt_db_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension/gatt_db_pl.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_a2dp.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_a2dp_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/a2dp_codec/sbc/a2dp_codec_sbc.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_api.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_internal.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_api.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_at.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_att_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avdtp_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_conn.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_conn_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_crypto.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_crypto_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_gatt_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_core.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_core.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_adv.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_adv.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_br.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_br.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_id.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_id.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_scan.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_scan.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ecc.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ecc.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_addr.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_buf.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_direction.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_direction_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_ecc.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys_br.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap_br.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_monitor.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_monitor.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rfcomm.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rfcomm_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_spp.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rpa.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rpa.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_settings.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_settings.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_sdp.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_sdp_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ssp.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ssp.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_uuid.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hfp_ag.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hfp_hf.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hfp_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/a2dp.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/avrcp.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/a2dp_codec_sbc.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/addr.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/common/addr.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/att.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/avdtp.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/bluetooth.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/direction.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/buf.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/l2cap.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/rfcomm.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/spp.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/sdp.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/conn.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/crypto.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/gap.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/gatt.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/hci.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/hci_err.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/hci_raw.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/hci_vs.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/hfp_hf.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/hfp_ag.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/iso.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/testing.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth/uuid.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/util.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/util_internal.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/util_loops.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/util_macro.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/printk.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/check.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/byteorder.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/zephyr/types.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/porting.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/porting.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/errno/errno.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/utf8.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/work_queue.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/work_queue.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/toolchain/common.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/toolchain/gcc.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/toolchain/iar.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/toolchain/armcc.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/toolchain.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/buf.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/net/buf.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/slist.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/slist.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/dlist.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting/atomic_c.c"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/include/sys/atomic.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller_wifi_nxp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller_hci_uart.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/edgefast/bluetooth/pal/bt_ble/configs
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/controller
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/controller/configs
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/vendor
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/att
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/avctp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/avdtp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/bnep
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/common
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/dbase
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/hci_1.2
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/mcap
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/obex
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/rfcomm
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/sdp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/sm
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/smp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/protocols/write_task
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/aes_cmac
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/at_parser
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/btsnoop
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/object_parser
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/racp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/security
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/status
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/storage
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/xml_parser
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/sbc
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/sbc
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/host
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/include
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/include/bluetooth
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/porting
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller
)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config_client.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config_client.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/devices/MIMXRT595S/drivers
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MIMXRT595S
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/middleware/sdmmc
    ${SdkRootDirPath}/core/drivers/trng
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/middleware/usb
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/middleware/fatfs
    ${SdkRootDirPath}/core/components/codec/wm8904
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/components/audio
    ${SdkRootDirPath}/core/components/gpio
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/devices/MIMXRT595S/utilities
    ${SdkRootDirPath}/middleware/wifi_nxp
    ${SdkRootDirPath}/core/components/common_task
    ${SdkRootDirPath}/core/boards/evkmimxrt595/flash_config
    ${SdkRootDirPath}/core/drivers/cache/cache64
    ${SdkRootDirPath}/core/drivers/lpc_iopctl
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/components/internal_flash/
    ${SdkRootDirPath}/core/components/pca9420
    ${SdkRootDirPath}/middleware/littlefs
    ${SdkRootDirPath}/core/drivers/inputmux
    ${SdkRootDirPath}/middleware/mbedtls
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/usdhc
    ${SdkRootDirPath}/core/drivers/i3c
    ${SdkRootDirPath}/core/drivers/lpc_dma
    ${SdkRootDirPath}/core/components/wifi_bt_module
    ${SdkRootDirPath}/core/components/log
    ${SdkRootDirPath}/core/drivers/casper
    ${SdkRootDirPath}/core/drivers/hashcrypt
)

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

include(middleware_sdmmc_host_usdhc_freertos_MIMXRT595S_cm33)

include(driver_trng)

include(driver_flexspi)

include(middleware_usb_host_stack_MIMXRT595S_cm33)

include(component_osa)

include(middleware_usb_host_ip3516hs_MIMXRT595S_cm33)

include(middleware_usb_phy)

include(middleware_fatfs_MIMXRT595S_cm33)

include(middleware_fatfs_usb)

include(driver_flexcomm_i2c)

include(driver_wm8904)

include(driver_codec)

include(component_wm8904_adapter)

include(component_codec_i2c_MIMXRT595S_cm33)

include(component_i3c_adapter)

include(driver_flexcomm_i2s)

include(driver_flexcomm_i2s_dma)

include(component_audio_flexcomm_i2s_dma_adapter)

include(component_rt_gpio_adapter)

include(middleware_freertos-kernel_heap_4)

include(utility_shell)

include(middleware_freertos-kernel_MIMXRT595S_cm33)

include(middleware_wifi_fwdnld_MIMXRT595S_cm33)

include(component_common_task)

include(driver_flash_config_evkmimxrt595)

include(driver_cache_cache64)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_lpc_gpio)

include(driver_reset)

include(component_flexspi_nor_flash_adapter_rt595evk)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_pca9420)

include(middleware_littlefs)

include(driver_inputmux)

include(middleware_mbedtls_MIMXRT595S_cm33)

include(middleware_mbedtls_port_ksdk)

include(middleware_mbedtls_rt1)

include(utilities_misc_utilities_MIMXRT595S_cm33)

include(CMSIS_Include_core_cm)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(middleware_freertos-kernel_extension)

include(driver_usdhc)

include(middleware_usb_common_header)

include(middleware_usb_host_common_header)

include(middleware_usb_host_msd)

include(driver_i3c)

include(driver_lpc_dma)

include(component_wifi_bt_module_tx_pwr_limits)

include(middleware_wifi_sdio-2)

include(middleware_wifi_common_files)

include(component_log_backend_debugconsole)

include(component_log)

include(driver_iap)

include(driver_inputmux_connections)

include(driver_casper)

include(driver_hashcrypt)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_a2dp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_avrcp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_bip.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_bpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_ctn.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_ftp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_ga.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_hfp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_hid.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_map.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_opp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_pan.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_pbap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_sap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_spp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_sbc_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_sbc_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_sbc_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_lc3_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_lc3_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_lc3_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_a2dp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_avrcp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_bip.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_bpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_ctn.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_ftp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_ga.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_hfp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_hid.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_map.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_opp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_pan.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_pbap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_sap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_spp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_bt_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_sbc_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_sbc_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_sbc_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_lc3_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_lc3_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm33/gcc/libethermind_lc3_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


