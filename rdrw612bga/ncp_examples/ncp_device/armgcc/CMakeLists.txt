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


project(ncp_device)

set(MCUX_BUILD_TYPES flash_release flash_debug)

set(MCUX_SDK_PROJECT_NAME ncp_device.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../system/ncp_system.h"
"${ProjDirPath}/../system/ncp_system.c"
"${ProjDirPath}/../system/ncp_glue_system.h"
"${ProjDirPath}/../system/ncp_glue_system.c"
"${ProjDirPath}/../wifi/ncp_wifi.c"
"${ProjDirPath}/../wifi/ncp_wifi.h"
"${ProjDirPath}/../wifi/ncp_glue_wifi.c"
"${ProjDirPath}/../wifi/ncp_glue_wifi.h"
"${ProjDirPath}/../wifi/wifi_config.h"
"${ProjDirPath}/../ble/gap.c"
"${ProjDirPath}/../ble/gatt.c"
"${ProjDirPath}/../ble/l2cap.c"
"${ProjDirPath}/../ble/ncp_ble.c"
"${ProjDirPath}/../ble/ncp_ble.h"
"${ProjDirPath}/../ble/ncp_glue_ble.c"
"${ProjDirPath}/../ble/ncp_glue_ble.h"
"${ProjDirPath}/../ble/app_bluetooth_config.h"
"${ProjDirPath}/../ble/service/gatt_server/peripheral_hrs.c"
"${ProjDirPath}/../ble/service/gatt_server/peripheral_hrs.h"
"${ProjDirPath}/../ble/service/gatt_server/peripheral_hts.c"
"${ProjDirPath}/../ble/service/gatt_server/peripheral_hts.h"
"${ProjDirPath}/../ble/service/gatt_client/central_hrc.c"
"${ProjDirPath}/../ble/service/gatt_client/central_hrc.h"
"${ProjDirPath}/../ble/service/gatt_client/central_htc.c"
"${ProjDirPath}/../ble/service/gatt_client/central_htc.h"
"${ProjDirPath}/../ble/service/bas.c"
"${ProjDirPath}/../ble/service/bas.h"
"${ProjDirPath}/../ble/service/service.c"
"${ProjDirPath}/../ble/service/service.h"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../lwippools.h"
"${ProjDirPath}/../lwiphooks.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../app_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_cmd_common.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_cmd_system.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_cmd_wifi.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_cmd_ble.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_glue_common.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/app_notify.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/app_notify.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_adapter.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_tlv/ncp_tlv_adapter.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_tlv/ncp_tlv_adapter.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/ncp_intf_pm.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/ncp_intf_pm.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/uart/ncp_intf_uart.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/uart/ncp_intf_uart.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_cdc_app.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_cdc_app.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/ncp_intf_usb_device_cdc.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/ncp_intf_usb_device_cdc.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_descriptor.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_descriptor.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_config.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/spi/spi_slave/ncp_intf_spi_slave.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/spi/spi_slave/ncp_intf_spi_slave.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/sdio/sdio_device/ncp_intf_sdio.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/sdio/sdio_device/ncp_intf_sdio.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_config/littlefs_config.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_config/littlefs_config.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_config/littlefs_adapter.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_config/littlefs_adapter.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_config/ncp_config.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_config/ncp_config.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/mdns/mdns_service.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/mdns/mdns_service.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/lpm/lpm.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/lpm/lpm.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/lpm/host_sleep.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/lpm/host_sleep.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/lpm/host_sleep_wifi.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/lpm/host_sleep_wifi.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/crc/crc.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/crc/crc.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_debug/ncp_debug.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/json_parser/jsmn.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/json_parser/json_generator.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/json_parser/json_parser.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/httpc.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/http-strings.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/serial_errno.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/serial_httpc.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/serial_network.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/serial_socket.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/websockets.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/wmstats.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/httpc/httpc.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/httpc/httpc_tests.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/json_parser/json_generator.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/serial_httpc/serial_httpc.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/serial_httpc/serial_network.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/serial_httpc/serial_socket.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/websocket/websockets.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/websocket/wslay.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/websocket/wslay_frame.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/websocket/wslay_frame.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/src/websocket/wslayver.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/uap_prov/httpsrv_fs_data.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/uap_prov/http_server.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/uap_prov/http_server.h"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/uap_prov/uap_prov.c"
"${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/uap_prov/uap_prov.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_class.c"
"${ProjDirPath}/../usb_device_class.h"
"${ProjDirPath}/../usb_device_cdc_acm.c"
"${ProjDirPath}/../usb_device_cdc_acm.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../ble
    ${ProjDirPath}/../ble/service
    ${ProjDirPath}/../ble/service/gatt_client
    ${ProjDirPath}/../ble/service/gatt_server
    ${ProjDirPath}/..
    ${ProjDirPath}/../system
    ${ProjDirPath}/../wifi
    ${SdkRootDirPath}/examples/rdrw612bga
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_tlv
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/uart
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/spi/spi_slave
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/sdio/sdio_device
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_config
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/mdns
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/lpm
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/crc
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_debug
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/httpc/inc/json_parser
    ${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/uap_prov
    ${SdkRootDirPath}/core/boards/rdrw612bga
)

set_source_files_properties("${ProjDirPath}/../wifi/wifi_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../ble/app_bluetooth_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_edgefast_bluetooth_config_template")
set_source_files_properties("${ProjDirPath}/../lwipopts.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_lwip_template")
set_source_files_properties("${ProjDirPath}/../app_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")
set_source_files_properties("${SdkRootDirPath}/examples/rdrw612bga/ncp_examples/common/ncp_adapter/ncp_intf/usb/usb_device_cdc/usb_device_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_usb_device_ehci_config_header")

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_config.h" MBEDTLS_USER_CONFIG_FILE="wpa_supp_els_pkc_mbedtls_config.h" LWIP_HOOK_FILENAME="lwip_default_hooks.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_config.h" MBEDTLS_USER_CONFIG_FILE="wpa_supp_els_pkc_mbedtls_config.h" LWIP_HOOK_FILENAME="lwip_default_hooks.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

include(${SdkRootDirPath}/core/devices/RW612/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin")

