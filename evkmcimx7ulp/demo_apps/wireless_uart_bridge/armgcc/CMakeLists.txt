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


project(wireless_uart_bridge)

set(MCUX_BUILD_TYPES release)

set(MCUX_SDK_PROJECT_NAME wireless_uart_bridge.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../app_config.c"
"${ProjDirPath}/../app_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../wireless_uart_bridge.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../ble/ApplMain.c"
"${ProjDirPath}/../ble/ApplMain.h"
"${ProjDirPath}/../ble/ble_conn_manager.c"
"${ProjDirPath}/../ble/ble_conn_manager.h"
"${ProjDirPath}/../ble/ble_service_discovery.c"
"${ProjDirPath}/../ble/ble_service_discovery.h"
"${ProjDirPath}/../ble/ble_init.h"
"${ProjDirPath}/../ble/framework/Common/EmbeddedTypes.h"
"${ProjDirPath}/../ble/framework/FSCI/Interface/FsciInterface.h"
"${ProjDirPath}/../ble/framework/FSCI/Source/FsciCommands.c"
"${ProjDirPath}/../ble/framework/FSCI/Source/FsciCommands.h"
"${ProjDirPath}/../ble/framework/FSCI/Source/FsciCommunication.c"
"${ProjDirPath}/../ble/framework/FSCI/Source/FsciCommunication.h"
"${ProjDirPath}/../ble/framework/FSCI/Source/FsciLogging.c"
"${ProjDirPath}/../ble/framework/FSCI/Source/FsciMain.c"
"${ProjDirPath}/../ble/framework/FunctionLib/FunctionLib.c"
"${ProjDirPath}/../ble/framework/FunctionLib/FunctionLib.h"
"${ProjDirPath}/../ble/framework/Lists/GenericList.h"
"${ProjDirPath}/../ble/framework/Lists/GenericList.c"
"${ProjDirPath}/../ble/framework/MemManager/Interface/MemManager.h"
"${ProjDirPath}/../ble/framework/MemManager/Source/MemManager.c"
"${ProjDirPath}/../ble/framework/Messaging/Source/Messaging.c"
"${ProjDirPath}/../ble/framework/Messaging/Interface/Messaging.h"
"${ProjDirPath}/../ble/framework/OSAbstraction/Source/fsl_os_abstraction_free_rtos.c"
"${ProjDirPath}/../ble/framework/OSAbstraction/Interface/fsl_os_abstraction.h"
"${ProjDirPath}/../ble/framework/OSAbstraction/Interface/fsl_os_abstraction_config.h"
"${ProjDirPath}/../ble/framework/OSAbstraction/Interface/fsl_os_abstraction_free_rtos.h"
"${ProjDirPath}/../ble/framework/Panic/Source/Panic.c"
"${ProjDirPath}/../ble/framework/Panic/Interface/Panic.h"
"${ProjDirPath}/../ble/framework/SerialManager/Interface/SerialManager.h"
"${ProjDirPath}/../ble/framework/SerialManager/Source/SerialManager.c"
"${ProjDirPath}/../ble/framework/SerialManager/Source/UART_Adapter.c"
"${ProjDirPath}/../ble/framework/SerialManager/Source/UART_Adapter.h"
"${ProjDirPath}/../ble/gatt_db/gatt_database.c"
"${ProjDirPath}/../ble/gatt_db/gatt_database_dynamic.c"
"${ProjDirPath}/../ble/gatt_db/gatt_database_dynamic.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_types.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_types.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gap.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gap.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gap_types.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gap_types.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt_types.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt_types.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt_db_app.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt_db_app.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt_db_app_types.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_gatt_db_app_types.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_l2cap.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_l2cap.h"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_l2cap_types.c"
"${ProjDirPath}/../ble/fsci_host/fsci/fsci_ble_l2cap_types.h"
"${ProjDirPath}/../ble/fsci_host/host/host_ble.c"
"${ProjDirPath}/../ble/fsci_host/host/host_ble.h"
"${ProjDirPath}/../ble/fsci_host/host/host_gap.c"
"${ProjDirPath}/../ble/fsci_host/host/host_gatt.c"
"${ProjDirPath}/../ble/fsci_host/host/host_gatt_db.c"
"${ProjDirPath}/../ble/fsci_host/host/host_l2cap.c"
"${ProjDirPath}/../ble/fsci_host/host/config/ble_config.h"
"${ProjDirPath}/../ble/fsci_host/host/config/ble_globals.c"
"${ProjDirPath}/../ble/fsci_host/host/interface/att_callbacks.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/att_errors.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/att_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/att_params.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/att_types.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/ble_constants.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/ble_general.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/ble_host_tasks.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/ble_sig_defines.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/ble_utils.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/fsci_ble_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gap_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gap_types.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_client_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_database.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_db_app_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_db_att_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_db_dynamic.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_server_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/gatt_types.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/hci_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/hci_types.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/l2ca_cb_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/l2ca_interface.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/l2ca_types.h"
"${ProjDirPath}/../ble/fsci_host/host/interface/vendor_debug_commands.h"
"${ProjDirPath}/../ble/profiles/device_info/device_info_interface.h"
"${ProjDirPath}/../ble/profiles/device_info/device_info_service.c"
"${ProjDirPath}/../ble/profiles/wireless_uart/wireless_uart_interface.h"
"${ProjDirPath}/../ble/profiles/wireless_uart/wireless_uart_service.c"
"${ProjDirPath}/../ble/profiles/battery/battery_interface.h"
"${ProjDirPath}/../ble/profiles/battery/battery_service.c"
"${ProjDirPath}/../rsc_table.c"
"${ProjDirPath}/../rsc_table.h"
"${ProjDirPath}/../remoteproc.h"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/board.c"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/board.h"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/clock_config.h"
"${ProjDirPath}/../srtm_config.h"
"${ProjDirPath}/../app_srtm.c"
"${ProjDirPath}/../app_srtm.h"
"${ProjDirPath}/../srtm/channels/srtm_rpmsg_endpoint.h"
"${ProjDirPath}/../srtm/channels/srtm_rpmsg_endpoint.c"
"${ProjDirPath}/../srtm/services/srtm_lfcl_service.h"
"${ProjDirPath}/../srtm/services/srtm_lfcl_service.c"
"${ProjDirPath}/../srtm/services/srtm_pmic_service.h"
"${ProjDirPath}/../srtm/services/srtm_pmic_service.c"
"${ProjDirPath}/../srtm/services/srtm_pf1550_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_pf1550_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_audio_service.h"
"${ProjDirPath}/../srtm/services/srtm_audio_service.c"
"${ProjDirPath}/../srtm/services/srtm_sai_edma_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_sai_edma_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_i2c_codec_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_i2c_codec_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_io_service.h"
"${ProjDirPath}/../srtm/services/srtm_io_service.c"
"${ProjDirPath}/../srtm/services/srtm_keypad_service.h"
"${ProjDirPath}/../srtm/services/srtm_keypad_service.c"
"${ProjDirPath}/../srtm/services/srtm_rtc_service.h"
"${ProjDirPath}/../srtm/services/srtm_rtc_service.c"
"${ProjDirPath}/../srtm/services/srtm_snvs_lp_rtc_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_snvs_lp_rtc_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_sensor_service.h"
"${ProjDirPath}/../srtm/services/srtm_sensor_service.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../ble
    ${ProjDirPath}/../ble/framework/Common
    ${ProjDirPath}/../ble/framework/FSCI/Interface
    ${ProjDirPath}/../ble/framework/FSCI/Source
    ${ProjDirPath}/../ble/framework/FunctionLib
    ${ProjDirPath}/../ble/framework/Lists
    ${ProjDirPath}/../ble/framework/MemManager/Interface
    ${ProjDirPath}/../ble/framework/Messaging/Interface
    ${ProjDirPath}/../ble/framework/OSAbstraction/Interface
    ${ProjDirPath}/../ble/framework/Panic/Interface
    ${ProjDirPath}/../ble/framework/SerialManager/Interface
    ${ProjDirPath}/../ble/framework/SerialManager/Source
    ${ProjDirPath}/../ble/gatt_db
    ${ProjDirPath}/../ble/fsci_host/fsci
    ${ProjDirPath}/../ble/fsci_host/host
    ${ProjDirPath}/../ble/fsci_host/host/config
    ${ProjDirPath}/../ble/fsci_host/host/interface
    ${ProjDirPath}/../ble/profiles/device_info
    ${ProjDirPath}/../ble/profiles/wireless_uart
    ${ProjDirPath}/../ble/profiles/battery
    ${SdkRootDirPath}/core/boards/evkmcimx7ulp
    ${ProjDirPath}/../srtm/channels
    ${ProjDirPath}/../srtm/services
)

set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")

include(${SdkRootDirPath}/core/devices/MCIMX7U5/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/wireless_uart_bridge.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/wireless_uart_bridge.bin")

