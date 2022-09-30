# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

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


project(mw_wifi_cli)

set(MCUX_SDK_PROJECT_NAME mw_wifi_cli.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../lwippools.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../network_flash_storage.c"
"${ProjDirPath}/../network_flash_storage.h"
"${ProjDirPath}/../lpm.c"
"${ProjDirPath}/../lpm.h"
"${ProjDirPath}/../../common/mw30x_uapsta_W14.88.36.p144.fw.bin"
"${ProjDirPath}/../../../sdmmc_config.c"
"${ProjDirPath}/../../../sdmmc_config.h"
"${ProjDirPath}/../wifi_config.h"
"${SdkRootDirPath}/core/boards/rdmw320_r0/board.c"
"${SdkRootDirPath}/core/boards/rdmw320_r0/board.h"
"${SdkRootDirPath}/core/boards/rdmw320_r0/clock_config.c"
"${SdkRootDirPath}/core/boards/rdmw320_r0/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/.. flash_debug
    ${ProjDirPath}/.. flash_debug
    ${SdkRootDirPath}/core/boards/rdmw320_r0 flash_debug
    ${SdkRootDirPath}/examples/rdmw320_r0/wifi_examples/common flash_debug
    ${ProjDirPath}/..
    ${ProjDirPath}/../../common
    ${ProjDirPath}/../../..
    ${SdkRootDirPath}/core/boards/rdmw320_r0
    ${SdkRootDirPath}/examples/rdmw320_r0/wifi_examples/common flash_release
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(component_mflash_file)

include(component_mflash_mw320)

include(component_boot2_psm)

include(middleware_freertos-kernel_88MW320)

include(middleware_freertos-kernel_heap_4)

include(middleware_lwip)

include(utility_debug_console)

include(middleware_wifi)

include(middleware_lwip_apps_lwiperf)

include(middleware_wifi_cli)

include(driver_mw_rtc)

include(driver_mw_crc)

include(driver_mw_sdioc)

include(middleware_sdmmc_sdio)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_sdioc)

include(middleware_sdmmc_host_sdioc_freertos)

include(driver_mw_aes)

include(driver_power)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_assert)

include(component_mw_uart_adapter)

include(driver_mw_qspi)

include(driver_mw_flashc)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_mw_uart)

include(device_startup)

include(driver_mw_pinmux)

include(driver_mw_gpio)

include(component_mflash_common)

include(CMSIS_Include_core_cm4)

include(CMSIS_Include_common)

include(CMSIS_Include_dsp)

include(component_boot2_partition)

include(middleware_freertos-kernel_extension)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin)

