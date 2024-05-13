# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION 2)
SET (MCUXPRESSO_CMAKE_FORMAT_MINOR_VERSION 0)

include(ide_overrides.cmake OPTIONAL)

if(CMAKE_SCRIPT_MODE_FILE)
  message("${MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION}")
  return()
endif()


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


project(coex_wpa_supplicant)

set(MCUX_BUILD_TYPES flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME coex_wpa_supplicant.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../app_config.h"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../lwippools.h"
"${ProjDirPath}/../lwiphooks.h"
"${ProjDirPath}/../mbedtls_config_client.h"
"${SdkRootDirPath}/examples/evkcmimxrt1060/board.c"
"${SdkRootDirPath}/examples/evkcmimxrt1060/board.h"
"${SdkRootDirPath}/examples/evkcmimxrt1060/clock_config.c"
"${SdkRootDirPath}/examples/evkcmimxrt1060/clock_config.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../wifi_bt_config.c"
"${ProjDirPath}/../wifi_bt_config.h"
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../appl/reference/FreeRTOSConfig.h"
"${ProjDirPath}/../appl/reference/appl_main.c"
"${ProjDirPath}/../appl/reference/appl_main.h"
"${ProjDirPath}/../appl/reference/appl_profile.h"
"${ProjDirPath}/../appl/reference/appl_protocol.h"
"${ProjDirPath}/../appl/reference/appl_utils.c"
"${ProjDirPath}/../appl/reference/appl_utils.h"
"${ProjDirPath}/../appl/lc3/sim/lc3_EM_debug.h"
"${ProjDirPath}/../appl/lc3/sim/lc3_EM_fops.h"
"${ProjDirPath}/../appl/lc3/sim/lc3_EM_os.h"
"${ProjDirPath}/../appl/lc3/sim/lc3_EM_platform.h"
"${ProjDirPath}/../appl/lc3/lc3_decoder_input_data.h"
"${ProjDirPath}/../appl/lc3/lc3_encoder_input_44100.h"
"${ProjDirPath}/../appl/lc3/lc3_encoder_input_48000.h"
"${ProjDirPath}/../appl/lc3/lc3_encoder_input_data.h"
"${ProjDirPath}/../appl/lc3/lc3_fops.h"
"${ProjDirPath}/../appl/lc3/lc3_fops.c"
"${ProjDirPath}/../appl/lc3/lc3_test.c"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../wifi_config.h"
"${ProjDirPath}/../a2dp_pl_ext.c"
"${ProjDirPath}/../wav.c"
"${ProjDirPath}/../wav.h"
"${ProjDirPath}/../controller_coex_nxp.c"
"${ProjDirPath}/../leaudio_pl_ext.c"
"${ProjDirPath}/../sco_audio_pl_ext.c"
"${ProjDirPath}/../ringtone.h"
"${ProjDirPath}/../db_gen.c"
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../host_msd_fatfs.c"
"${ProjDirPath}/../host_msd_fatfs.h"
"${ProjDirPath}/../usb_host_config.h"
"${ProjDirPath}/../coex.c"
"${ProjDirPath}/../coex.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_a2dp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_a2dp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_bip.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_bip_initiator.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_bip_responder.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_bpp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_bpp_printer.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_bpp_sender.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_ctn.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_ctn_cce.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_ctn_cse.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_did.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_did_client.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_did_server.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_ftp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_ftp_client.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_ftp_server.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hdp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hdp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hfp_ag.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hfp_ag.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hfp_ag_atrsp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hfp_unit.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hfp_unit.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hid_device.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hid_device.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hid_host.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_hid_host.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_map.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_map_mce.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_map_mse.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_opp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_opp_client.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_opp_server.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_pbap_pce.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_pbap_pce.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_pbap_pse.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_pbap_pse.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_sap_client.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_sap_client.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_sap_server.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_sap_server.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_spp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_spp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_pan.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl/appl_pan.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_bac.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_bpc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_cpmc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_cscc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_dic.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_fml.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_glc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_hidh.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_hrc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_htc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_pasp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_pxm.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_rscc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_hpc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_tic.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client/appl_ipspr.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller_features.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller_features.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hps_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hps_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/https_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/https_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ipsp_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ipsp_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ipspr_ping.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/niface.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/niface.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/pan_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/pan_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/appl_hps.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/appl_hps.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/appl_hps_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/appl_hps_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/appl_hps_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/gatt_db_dyn_hps.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/gatt_db_hps.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/gatt_db_hps_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps/gatt_db_stat_hps.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/appl_hts.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/appl_hts.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/appl_hts_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/appl_hts_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/appl_hts_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/gatt_db_dyn_hts.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/gatt_db_hts.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/gatt_db_hts_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts/gatt_db_stat_hts.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/appl_ipspn.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/appl_ipspn.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/appl_ipspn_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/appl_ipspn_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/appl_ipspn_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/gatt_db_dyn_ipspn.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/gatt_db_ipspn.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/gatt_db_ipspn_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn/gatt_db_stat_ipspn.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/appl_pxr.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/appl_pxr.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/appl_pxr_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/appl_pxr_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/appl_pxr_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/gatt_db_dyn_pxr.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/gatt_db_pxr.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/gatt_db_pxr_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr/gatt_db_stat_pxr.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_assert.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_config.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_debug_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_fops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_serial.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/eOSAL/eBT_timer.h"
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
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/at_parser/at_parser_hsp_hfp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/storage/storage.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_common_defines.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_encoder.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/xml_parser/xml_parser.h"
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
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ctn_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ctn_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/cry_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/cry_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ftp_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ftp_pl.h"
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
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/fw_loader_uart.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/fw_loader_uart.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/sbc/sbc_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/lc3/LC3_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/EM_platform.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/EM_platform.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_a2dp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/include/BT_analyzer.h"
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
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/aes_cmac/aes_cmac.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/object_parser/object_parser_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/object_parser/tokenizer.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/racp/racp.h"
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
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/sbc/sbc_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/controller_hci_uart.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_avctp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_avctp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_avdtp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_avdtp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_bnep.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_bnep.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_dbase.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_dbase_dyn.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_hci.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_hci.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_hci_api.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_l2cap.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_l2cap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_mcap.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_mcap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_obex_client.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_obex_client.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_obex_common.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_obex_server.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_obex_server.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_rfcomm.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_rfcomm.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_sdp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_sdp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_sm.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl/appl_sm.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp_ct.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp_ct.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp_tg.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp_tg.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp_tg_virtual_media_player.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/appl_avrcp_tg_virtual_media_player.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/avrcp_al_api.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/avrcp_al_internal.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/avrcp_al_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/BT_avrcp_al_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al/avrcp_al_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_aics.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_aics.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_bass.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_bass.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_csip.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_csip.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ccp_ce.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ccp_ce.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ccp_se.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ccp_se.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ccp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_micp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_micp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_vcp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_vcp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_mcp_ce.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_mcp_ce.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_mcp_se.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_mcp_se.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/otp_se_utils.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/audio_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/audio_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/leaudio_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/leaudio_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_hap.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_hap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_tmap.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_tmap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_bc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_bc.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_bc_ba.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_bc_ba.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_bc_sd.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_bc_sd.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_common.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_features.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_legacy.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_legacy_odsp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_legacy.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_audio_snk.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_audio_snk_odsp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_audio_snk.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_audio_src_odsp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_audio_src.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_audio_src.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_uc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_uc.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_ucc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_ucc.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_ucs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_ucs.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_hci.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ga_utils_hci.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ccp_utils.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_ccp_utils.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_mcp_utils.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl/appl_mcp_utils.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_api.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_appl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_init.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_internal.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_utils.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_brr_utils.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_gatt_db.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_gatt_db.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer/ga_gatt_db_dyn.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/eOSAL/GA_os.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/eOSAL/GA_timer.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/eOSAL/GA_debug.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_hap_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_tmap_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_assigned_numbers.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_audio_config.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_bearer_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_ccp_ce_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_ccp_se_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_common.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_error.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_features.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_limits.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_mcp_ce_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_mcp_se_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_property_config.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_bass_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_bass_db.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include/GA_pbp_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/bap/bap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/bap/bass/bass_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/bap/bass/bass_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/csip/csip.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/csip/csip_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/csip/csip_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/micp/micp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/micp/micp_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/micp/micp_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/vcp/vcp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/vcp/vcp_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/vcp/vcp_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/l2cap_coc.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/otc.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/otc_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/otc_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/otc_transport.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/otp_common.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/ots.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/ots_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/ots_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp/ots_transport.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/cap.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/cap_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/cap_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/ccp/ccp_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/ccp/ccp_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/mcp/mcp_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/mcp/mcp_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/brr/brr_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/hap/hap_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/hap/hap_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/tmap/tmap_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/tmap/tmap_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/lc3/mcuxpresso/intrinsics_map.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/lc3/mcuxpresso/lc3_common_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/export/include/LC3_api.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/private/fixed/common/lc3_defines.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/private/fixed/common/lc3_extern.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/private/fixed/common/lc3_internal.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/private/fixed/config/LC3_features.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/private/fixed/config/LC3_limits.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/appl_le.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/appl_le.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_att_client.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_att_client.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_gap_le.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_gap_le.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_hci_le.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_hci_le.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_hci_le_api.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_l2cap_le.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_l2cap_le.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_l2cap_ecbfc.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_l2cap_ecbfc.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_smp.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl/appl_smp.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/appl_gatt_client.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/appl_gatt_client.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/appl_gatt_server.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/appl_gatt_server.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/gatt.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/gatt.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/appl_beacon.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/appl_beacon.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hfp_ag_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hfp_ag_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/nwsim.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/nwsim.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/appl_service.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/appl_service.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/appl_service_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/appl_service_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gatt_db.h"
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
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/a2dp_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/a2dp_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/a2dp_pl_media_16KHz.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/a2dp_pl_media_48KHz.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/vcom_pl.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/vcom_pl.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/appl_bps.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/appl_bps.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/appl_bps_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/appl_bps_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/appl_bps_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/gatt_db_bps.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/gatt_db_bps_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/gatt_db_dyn_bps.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps/gatt_db_stat_bps.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/appl_cpms.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/appl_cpms.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/appl_cpms_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/appl_cpms_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/appl_cpms_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/gatt_db_cpms.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/gatt_db_cpms_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/gatt_db_dyn_cpms.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms/gatt_db_stat_cpms.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/appl_cscs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/appl_cscs.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/appl_cscs_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/appl_cscs_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/appl_cscs_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/gatt_db_cscs.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/gatt_db_cscs_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/gatt_db_dyn_cscs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs/gatt_db_stat_cscs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/appl_gls.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/appl_gls.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/appl_gls_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/appl_gls_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/appl_gls_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/gatt_db_dyn_gls.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/gatt_db_gls.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/gatt_db_gls_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls/gatt_db_stat_gls.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/appl_basic.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/appl_basic.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/appl_basic_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/appl_basic_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/appl_basic_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/gatt_db_basic.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/gatt_db_basic_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/gatt_db_dyn_basic.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic/gatt_db_stat_basic.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/appl_hid.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/appl_hid.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/appl_hid_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/appl_hid_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/appl_hid_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/gatt_db_dyn_hid.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/gatt_db_hid.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/gatt_db_hid_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid/gatt_db_stat_hid.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/appl_hrs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/appl_hrs.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/appl_hrs_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/appl_hrs_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/appl_hrs_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/gatt_db_dyn_hrs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/gatt_db_hrs.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/gatt_db_hrs_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs/gatt_db_stat_hrs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/appl_tis.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/appl_tis.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/appl_tis_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/appl_tis_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/appl_tis_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/gatt_db_dyn_tis.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/gatt_db_stat_tis.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/gatt_db_tis.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis/gatt_db_tis_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/appl_pts.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/appl_pts.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/appl_pts_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/appl_pts_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/appl_pts_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/gatt_db_dyn_pts.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/gatt_db_pts.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/gatt_db_pts_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts/gatt_db_stat_pts.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/appl_pass.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/appl_pass.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/appl_pass_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/appl_pass_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/appl_pass_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/gatt_db_dyn_pass.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/gatt_db_pass.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/gatt_db_pass_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass/gatt_db_stat_pass.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/appl_rscs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/appl_rscs.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/appl_rscs_gap_config_params.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/appl_rscs_mops.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/appl_rscs_mops.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/gatt_db_dyn_rscs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/gatt_db_rscs.h"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/gatt_db_rscs_handler.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs/gatt_db_stat_rscs.c"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/a2dp/SineWaveMinus48.wav"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/a2dp/a2dp_source.conf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/a2dp/sample.wav"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/Image_200_200.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/Image_640_480.gif"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/Image_640_480.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/Image_75_75.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_cap_company_id_list.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_cap_event_id_list.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_folder_depth_1_items_list.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_media_element_attr_info.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_media_element_attr_info_large.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_media_element_attr_info_new_track.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_media_element_list_file.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_media_player_list_info.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_new_media_element_file.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_player_appl_attr_info.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_player_cur_media_play_status.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/avrcp_root_folder_items_list.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/company_id_list_1.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/event_id_list_1.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/folder_items_list.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/media_element_list_file.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/media_player_list_file.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/new_folder_item.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/new_song_info.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/player_attr_file_1.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/player_status_file.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/player_vfs.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/song_info_1.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp/song_info_1_for_fragmentation_handling.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/BIP_1_160_120.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/BIP_1_160_120_jpg_desc.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/BIP_1_160_120_thumbnail.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/BIP_1_640_480.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/BIP_1_640_480_jpg_desc.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/BIP_1_640_480_thumbnail.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/image_list.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/image_properties.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/images_capabilities.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/monitoring_image.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/sample.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/sample1.png"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/sample2.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/sample3.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip/sample_thumbnail.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/bigpic.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/feynman.jpg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/sample_ref_push_file.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/vCal_multiple_object.vcs"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/vCal_single_object.vcs"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/vCard_multiple.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/vCard_single.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp/vmsg.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/CTN-account-info.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/CTN-event.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/CTN-listing_large.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/new_calendar.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/push_calendar.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/push_notes.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/push_task.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/calendar/10000000000000000000000000000001.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/calendar/10000000000000000000000000000002.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/calendar/10000000000000000000000000000003.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/calendar/10000000000000000000000000000004.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/calendar/10000000000000000000000000000005.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/calendar/10000000000000000000000000000006.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/calendar/CTN-listing.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/notes/20000000000000000000000000000001.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/notes/20000000000000000000000000000002.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/notes/CTN-listing.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/tasks/30000000000000000000000000000001.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/tasks/30000000000000000000000000000002.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/tasks/30000000000000000000000000000003.ics"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn/root/telecom/CTN/tasks/CTN-listing.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ftp/food.jpeg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ftp/sachin.jpeg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ftp/sample.bmp"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/event_email.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/event_im.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/event_mms.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/event_sms_cdma.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/event_sms_gsm.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/map_event_report_v11.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/map_event_report_v12.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/map_mse_participant_data.xml"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_email.msg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_email.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_im.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_mms.msg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_mms.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_sms_cdma.msg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_sms_cdma.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_sms_gsm.msg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map/message_sms_gsm.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/bmp2MB.bmp"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/mybmp.bmp"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/mycal.vcs"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/mycard.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/mymsg.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/mynote.vnt"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vCal-1.vcs"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vCal-2.vcs"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vCal-multiple.vcs"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vCard-1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vCard-2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vCard-multiple.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vMsg-1.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vMsg-2.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vMsg-multiple.vmg"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vNote-1.vnt"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vNote-2.vnt"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp/vNote-multiple.vnt"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/arp_gratuitous_reply.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/arp_request.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/arp_response.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/dhcp_ack.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/dhcp_discover.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/dhcp_offer.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/dhcp_request.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/dns_query.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/dns_response.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/eth_payload_60_0x34d.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/eth_payload_60_0x44a.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/eth_payload_60_0x5a0.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/eth_payload_ipv4ping.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/http_request.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/http_request_ipv6.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/http_response.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/http_response_ipv6.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/llmnr_query.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/llmnr_query_ipv6.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/llmnr_response.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/ndns_request.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/ping_reply.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/ping_reply_ipv6.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/ping_request.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/ping_request_ipv6.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/wap_request.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan/wap_response.dat"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/pb_0.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/pb_1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/cch/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/cch/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/cch/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/cch/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/cch/5.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/cch/6.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/cch/7.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/fav/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/fav/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/fav/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/mch/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/mch/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/mch/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/mch/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/och/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/och/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/och/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/och/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/och/5.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/0.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/10.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/11.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/12.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/13.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/14.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/15.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/16.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/17.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/5.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/6.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/7.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/8.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/pb/9.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/spd/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/spd/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/SIM1/telecom/spd/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/16.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/17.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/5.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/6.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/cch/7.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/fav/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/fav/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/fav/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/ich/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/ich/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/ich/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/ich/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/ich/5.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/ich/6.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/mch/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/mch/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/mch/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/mch/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/och/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/och/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/och/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/och/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/och/5.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/0.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/10.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/11.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/12.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/13.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/14.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/15.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/16.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/17.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/3.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/4.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/5.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/6.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/7.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/8.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/pb/9.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/spd/1.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/spd/2.vcf"
"${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap/phonebook/root/telecom/spd/3.vcf"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${SdkRootDirPath}/examples/evkcmimxrt1060
    ${ProjDirPath}/..
    ${ProjDirPath}/../appl/reference
    ${ProjDirPath}/../appl/lc3
    ${ProjDirPath}/../appl/lc3/sim
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/profile_appl
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/client
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hps
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hts
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/ipspn
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pxr
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
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/ethal
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/sbc
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/lc3
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/osal/src/freertos
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/sbc
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/protocol_appl
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/br/al
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/appl
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/ga/bearer
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/eOSAL
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/include
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/ccp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/profiles/mcp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/bap
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/bap/bass
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/csip
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/micp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/vcp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/profiles/ga/source/core/otp
    ${SdkRootDirPath}/middleware/wireless/ethermind/examples/ethermind
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/toolspec/iar
    ${SdkRootDirPath}/middleware/wireless/ethermind/port/pal/mcux/lc3/mcuxpresso
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/export/include
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/private/fixed/common
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/utils/lc3/private/fixed/config
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/protocol_appl
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/extension
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/bps
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cpms
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/cscs
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/gls
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/basic
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hid
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/hrs
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/tis
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pts
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/pass
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/export/appl/reference/le/profile_appl/server/rscs
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/a2dp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/avrcp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bip
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/bpp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ctn
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/ftp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/map
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/opp
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pan
    ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/tools/resources/pbap
)

set_source_files_properties("${ProjDirPath}/../app_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../lwipopts.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_lwip_template")
set_source_files_properties("${ProjDirPath}/../sdmmc_config.c" PROPERTIES COMPONENT_CONFIG_FILE "middleware_sdmmc_usdhc_template")
set_source_files_properties("${ProjDirPath}/../sdmmc_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_sdmmc_usdhc_template")
set_source_files_properties("${ProjDirPath}/../wifi_bt_config.c" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../wifi_bt_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../appl/reference/FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")
set_source_files_properties("${ProjDirPath}/../wifi_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../ffconf.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_fatfs_template_usb")
set_source_files_properties("${ProjDirPath}/../usb_host_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_usb_host_ehci_config_header")

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config_client.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config_client.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

include(${SdkRootDirPath}/core/devices/MIMXRT1062/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_a2dp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_avrcp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_bip.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_bpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_ctn.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_ftp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_ga.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_hfp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_hid.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_map.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_opp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_pan.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_pbap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_sap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_spp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_sbc_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_sbc_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_sbc_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_odsp_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_odsp_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_odsp_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_a2dp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_avrcp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_bip.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_bpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_ctn.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_ftp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_ga.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_hfp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_hid.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_map.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_opp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_pan.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_pbap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_sap.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_spp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_bt_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_sbc_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_sbc_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_sbc_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_odsp_common.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_odsp_decoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/btdm/cm7f/gcc/libethermind_lc3_odsp_encoder.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map")

