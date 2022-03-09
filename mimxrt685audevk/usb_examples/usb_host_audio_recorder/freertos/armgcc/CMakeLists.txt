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


project(host_audio_recorder_freertos)

set(MCUX_SDK_PROJECT_NAME host_audio_recorder_freertos.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../app.c"
"${ProjDirPath}/../audio_recorder.c"
"${ProjDirPath}/../audio_recorder.h"
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../usb_host_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../../core/boards/mimxrt685audevk
    ${ProjDirPath}/../../../../../../examples/mimxrt685audevk/usb_examples/usb_host_audio_recorder/freertos
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_usb_host_ip3516hs_MIMXRT685S_cm33)

include(middleware_usb_phy)

include(driver_lpc_gpio)

include(driver_usdhc)

include(middleware_sdmmc_sd)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_freertos_MIMXRT685S_cm33)

include(middleware_fatfs)

include(middleware_fatfs_sd)

include(driver_flexcomm_i2c)

include(driver_pca9420)

include(middleware_freertos-kernel_heap_4)

include(middleware_freertos-kernel_MIMXRT685S_cm33)

include(middleware_usb_host_audio)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

include(driver_power)

include(driver_flash_config_mimxrt685audevk)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_flexcomm_usart)

include(driver_cache_cache64)

include(driver_flexspi)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_reset)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(middleware_usb_host_stack_MIMXRT685S_cm33)

include(component_osa)

include(middleware_usb_common_header)

include(CMSIS_Include_core_cm)

include(middleware_usb_host_common_header)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

