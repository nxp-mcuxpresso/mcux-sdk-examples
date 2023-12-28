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


project(peripheral_ht)

set(MCUX_BUILD_TYPES flexspi_nor_release flexspi_nor_debug)

set(MCUX_SDK_PROJECT_NAME peripheral_ht.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../app_config.h"
"${ProjDirPath}/../app_bluetooth_config.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../wifi_bt_config.c"
"${ProjDirPath}/../wifi_bt_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../controller/configs/mbedtls/mbedtls_config_client.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../peripheral_ht.c"
"${ProjDirPath}/../peripheral_ht.h"
"${ProjDirPath}/../edgefast_bluetooth_config.h"
"${ProjDirPath}/../edgefast_bluetooth_debug_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../wifi_bt_module_config.h"
"${SdkRootDirPath}/core/boards/evkcmimxrt1060/dcd.c"
"${SdkRootDirPath}/core/boards/evkcmimxrt1060/dcd.h"
"${ProjDirPath}/../controller/configs/lwip/lwipopts.h"
"${ProjDirPath}/../controller/configs/lwip/lwippools.h"
"${ProjDirPath}/../controller/configs/wifi/wifi_config.h"
"${ProjDirPath}/../platform/configs/ffconf.h"
"${ProjDirPath}/../platform/configs/usb_host_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../controller/configs/mbedtls
    ${SdkRootDirPath}/core/boards/evkcmimxrt1060
    ${ProjDirPath}/../controller/configs/lwip
    ${ProjDirPath}/../controller/configs/wifi
    ${ProjDirPath}/../platform/configs
)

set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")
set_source_files_properties("${ProjDirPath}/../app_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../app_bluetooth_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_edgefast_bluetooth_config_template")
set_source_files_properties("${ProjDirPath}/../sdmmc_config.c" PROPERTIES COMPONENT_CONFIG_FILE "middleware_sdmmc_usdhc_template")
set_source_files_properties("${ProjDirPath}/../sdmmc_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_sdmmc_usdhc_template")
set_source_files_properties("${ProjDirPath}/../wifi_bt_config.c" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../wifi_bt_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../controller/configs/mbedtls/mbedtls_config_client.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_edgefast_bluetooth_template")
set_source_files_properties("${ProjDirPath}/../edgefast_bluetooth_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_edgefast_bluetooth_config_template")
set_source_files_properties("${ProjDirPath}/../edgefast_bluetooth_debug_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_edgefast_bluetooth_config_template")
set_source_files_properties("${ProjDirPath}/../controller/configs/wifi/wifi_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../platform/configs/ffconf.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_fatfs_template_usb")
set_source_files_properties("${ProjDirPath}/../platform/configs/usb_host_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_usb_host_ehci_config_header")

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="ksdk_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="ksdk_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

include(${SdkRootDirPath}/core/devices/MIMXRT1062/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map")

