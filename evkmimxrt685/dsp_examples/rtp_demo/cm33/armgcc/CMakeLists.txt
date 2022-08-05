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


project(dsp_rtp_demo_cm33)

set(MCUX_SDK_PROJECT_NAME dsp_rtp_demo_cm33.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../message.h"
"${ProjDirPath}/../rtp.h"
"${ProjDirPath}/../dsp_ipc.c"
"${ProjDirPath}/../dsp_ipc.h"
"${ProjDirPath}/../user_config.h"
"${ProjDirPath}/../main_cm33.c"
"${ProjDirPath}/../main_cm33.h"
"${ProjDirPath}/../app_dsp_ipc.c"
"${ProjDirPath}/../app_dsp_ipc.h"
"${ProjDirPath}/../rtp_buffer.c"
"${ProjDirPath}/../rtp_buffer.h"
"${ProjDirPath}/../rtp_receiver.c"
"${ProjDirPath}/../rtp_receiver.h"
"${ProjDirPath}/../wifi_client.c"
"${ProjDirPath}/../wifi_client.h"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../dsp_config.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../incbin.S"
"${ProjDirPath}/../wifi_config.h"
"${ProjDirPath}/../dsp_support.c"
"${ProjDirPath}/../dsp_support.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../../../../sdmmc_config.c"
"${ProjDirPath}/../../../../sdmmc_config.h"
"${ProjDirPath}/../../../../wifi_bt_config.c"
"${ProjDirPath}/../../../../wifi_bt_config.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${ProjDirPath}/../app_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.h"
"${ProjDirPath}/../../dsp/binary/dsp_text_release.bin"
"${ProjDirPath}/../../dsp/binary/dsp_data_release.bin"
"${ProjDirPath}/../../dsp/binary/dsp_ncache_release.bin"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/evkmimxrt685/dsp_examples/rtp_demo/dsp/binary
    ${SdkRootDirPath}/examples/evkmimxrt685/dsp_examples/rtp_demo/cm33
    ${SdkRootDirPath}/core/boards/evkmimxrt685
    ${SdkRootDirPath}/examples/evkmimxrt685
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

include(driver_lpc_gpio)

include(middleware_lwip)

include(middleware_wifi)

include(driver_common)

include(driver_power)

include(driver_reset)

include(driver_dsp)

include(driver_inputmux)

include(middleware_multicore_rpmsg_lite_evkmimxrt685_freertos)

include(driver_mu)

include(driver_pca9420)

include(driver_wm8904)

include(driver_codec)

include(component_wm8904_adapter)

include(component_codec_i2c_MIMXRT685S_cm33)

include(component_i3c_adapter)

include(driver_i3c)

include(middleware_wifi_wifidriver)

include(middleware_freertos-kernel_MIMXRT685S_cm33)

include(utility_debug_console)

include(middleware_wifi_sdio)

include(middleware_wifi_fwdnld)

include(middleware_sdmmc_sdio)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_freertos_MIMXRT685S_cm33)

include(component_rt_gpio_adapter)

include(device_CMSIS)

include(utility_assert)

include(component_usart_adapter)

include(driver_flash_config_evkmimxrt685)

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

include(middleware_freertos-kernel_extension)

include(component_wifi_bt_module_tx_pwr_limits)

include(driver_inputmux_connections)

include(middleware_freertos-kernel_heap_4)

include(middleware_wifi_sdio-2)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(driver_usdhc)

include(utilities_misc_utilities_MIMXRT685S_cm33)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

