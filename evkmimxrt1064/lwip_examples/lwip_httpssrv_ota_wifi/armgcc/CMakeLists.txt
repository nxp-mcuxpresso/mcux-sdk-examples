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


project(lwip_httpssrv_ota_wifi)

set(MCUX_BUILD_TYPES flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME lwip_httpssrv_ota_wifi.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../lwip_httpssrv_ota.c"
"${ProjDirPath}/../httpsrv_multipart.c"
"${ProjDirPath}/../httpsrv_multipart.h"
"${ProjDirPath}/../network_cfg.h"
"${ProjDirPath}/../init_wifi.c"
"${ProjDirPath}/../wpl.h"
"${ProjDirPath}/../wpl_nxp.c"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../httpsrv_fs_data.c"
"${ProjDirPath}/../mbedtls_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../wifi_config.h"
"${ProjDirPath}/../mcuboot_app_support.c"
"${ProjDirPath}/../mcuboot_app_support.h"
"${ProjDirPath}/../bootutil_port/board_specific/flash_partitioning.h"
"${ProjDirPath}/../bootutil_port/board_specific/flash_partitioning.c"
"${ProjDirPath}/../bootutil_port/board_specific/sblconfig.h"
"${ProjDirPath}/../bootutil_port/mcuboot_config/mcuboot_config.h"
"${ProjDirPath}/../bootutil_port/mcuboot_config/mcuboot_logging.h"
"${ProjDirPath}/../bootutil_port/flash_map.h"
"${ProjDirPath}/../bootutil_port/sysflash/sysflash.h"
"${ProjDirPath}/../bootutil_port/sbldef.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../wifi_bt_config.c"
"${ProjDirPath}/../wifi_bt_config.h"
"${ProjDirPath}/../app_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1064/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1064/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1064/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1064/clock_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1064/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1064/dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${ProjDirPath}/../bootutil_port
    ${ProjDirPath}/../bootutil_port/mcuboot_config
    ${ProjDirPath}/../bootutil_port/sysflash
    ${ProjDirPath}/../bootutil_port/board_specific
    ${SdkRootDirPath}/examples/evkmimxrt1064
    ${SdkRootDirPath}/core/boards/evkmimxrt1064
)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/drivers/trng
    ${SdkRootDirPath}/core/drivers/dcp
    ${SdkRootDirPath}/core/drivers/cache/armv7-m7
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/components/flash/mflash/mimxrt1064
    ${SdkRootDirPath}/middleware/mbedtls
    ${SdkRootDirPath}/middleware/wifi_nxp
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/middleware/sdmmc
    ${SdkRootDirPath}/core/components/gpio
    ${SdkRootDirPath}/middleware/lwip
    ${SdkRootDirPath}/core/devices/MIMXRT1064/drivers
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MIMXRT1064
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/components/flash/mflash
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/core/drivers/usdhc
    ${SdkRootDirPath}/core/components/wifi_bt_module
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_trng)

include(driver_dcp)

include(driver_cache_armv7_m7)

include(driver_flexspi)

include(component_mflash_rt1064)

include(middleware_mbedtls_MIMXRT1064)

include(middleware_mbedtls_port_ksdk)

include(middleware_mbedtls_rt)

include(middleware_wifi_wifidriver)

include(middleware_freertos-kernel_MIMXRT1064)

include(utility_debug_console)

include(middleware_wifi_sdio)

include(middleware_wifi_fwdnld_MIMXRT1064)

include(middleware_sdmmc_sdio)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_freertos_MIMXRT1064)

include(component_igpio_adapter)

include(middleware_lwip_apps_httpssrv)

include(driver_clock)

include(middleware_freertos-kernel_heap_3)

include(middleware_lwip)

include(middleware_wifi)

include(driver_common)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(CMSIS_Include_core_cm)

include(component_mflash_common)

include(middleware_freertos-kernel_extension)

include(middleware_wifi_sdio-2)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(driver_usdhc)

include(component_wifi_bt_module_tx_pwr_limits)

include(middleware_wifi_common_files)

include(middleware_lwip_apps_httpsrv)

include(utilities_misc_utilities_MIMXRT1064)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/lwip_httpssrv_ota.bin)

