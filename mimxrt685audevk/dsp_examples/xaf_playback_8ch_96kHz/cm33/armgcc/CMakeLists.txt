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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(dsp_xaf_playback_8ch_96kHz_cm33)

set(MCUX_SDK_PROJECT_NAME dsp_xaf_playback_8ch_96kHz_cm33.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_cm33.c"
"${ProjDirPath}/../main_cm33.h"
"${ProjDirPath}/../cmd.c"
"${ProjDirPath}/../cmd.h"
"${ProjDirPath}/../dsp_ipc.c"
"${ProjDirPath}/../dsp_ipc.h"
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../dsp_config.h"
"${ProjDirPath}/../srtm_config.h"
"${ProjDirPath}/../srtm_config_audio.h"
"${ProjDirPath}/../srtm_config_nn.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../incbin.S"
"${ProjDirPath}/../dsp_support.c"
"${ProjDirPath}/../dsp_support.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.h"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.h"
"${ProjDirPath}/../../dsp/binary/dsp_text_release.bin"
"${ProjDirPath}/../../dsp/binary/dsp_data_release.bin"
"${ProjDirPath}/../../dsp/binary/dsp_ncache_release.bin"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/mimxrt685audevk/dsp_examples/xaf_playback_8ch_96kHz/dsp/binary
    ${SdkRootDirPath}/examples/mimxrt685audevk/dsp_examples/xaf_playback_8ch_96kHz/cm33
    ${SdkRootDirPath}/core/boards/mimxrt685audevk
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_clock)

include(driver_flexcomm_i2c)

include(middleware_multicore_rpmsg_lite_MIMXRT685S_cm33)

include(middleware_multicore_rpmsg_lite_freertos)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(utility_shell)

include(driver_lpc_gpio)

include(driver_usdhc)

include(middleware_sdmmc_sd)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_freertos_MIMXRT685S_cm33)

include(middleware_fatfs)

include(middleware_fatfs_sd)

include(driver_common)

include(driver_power)

include(driver_reset)

include(driver_dsp)

include(driver_inputmux)

include(middleware_multicore_rpmsg_lite_mimxrt685audevk_freertos)

include(driver_mu)

include(driver_pca9420)

include(driver_cs42448)

include(driver_codec)

include(component_cs42448_adapter)

include(component_codec_i2c_MIMXRT685S_cm33)

include(component_flexcomm_i2c_adapter)

include(component_rt_gpio_adapter)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

include(driver_flash_config_mimxrt685audevk)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_flexcomm_usart)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_MIMXRT685S_cm33)

include(middleware_freertos-kernel_extension)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(driver_inputmux_connections)

include(middleware_freertos-kernel_heap_4)

include(utilities_misc_utilities_MIMXRT685S_cm33)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

