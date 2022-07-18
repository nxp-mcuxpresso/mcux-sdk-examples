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


project(dsp_nn_demo_cm33)

set(MCUX_SDK_PROJECT_NAME dsp_nn_demo_cm33.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_cm33.c"
"${ProjDirPath}/../counter.c"
"${ProjDirPath}/../counter.h"
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
"${ProjDirPath}/../dsp_ipc.c"
"${ProjDirPath}/../dsp_ipc.h"
"${ProjDirPath}/../dsp_nn_utils.c"
"${ProjDirPath}/../dsp_nn_utils.h"
"${ProjDirPath}/../dsp_nn.c"
"${ProjDirPath}/../dsp_nn.h"
"${ProjDirPath}/../config_nn.h"
"${ProjDirPath}/../unit_tests_nn.h"
"${ProjDirPath}/../unit_tests_nn.c"
"${ProjDirPath}/../unit_tests_nn_conv_ds.c"
"${ProjDirPath}/../conv_ds_in.h"
"${ProjDirPath}/../conv_ds_out.h"
"${ProjDirPath}/../unit_tests_nn_conv_std.c"
"${ProjDirPath}/../conv_std_in.h"
"${ProjDirPath}/../conv_std_out.h"
"${ProjDirPath}/../unit_tests_nn_maxpool.c"
"${ProjDirPath}/../maxpool_in.h"
"${ProjDirPath}/../maxpool_out.h"
"${ProjDirPath}/../unit_tests_nn_relu.c"
"${ProjDirPath}/../unit_tests_nn_echo.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.h"
"${ProjDirPath}/../../dsp/binary/dsp_text_release.bin"
"${ProjDirPath}/../../dsp/binary/dsp_data_release.bin"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../dsp/binary
    ${SdkRootDirPath}/examples/evkmimxrt685/dsp_examples/nn_demo/cm33
    ${SdkRootDirPath}/core/boards/evkmimxrt685
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

include(driver_i3c)

include(middleware_multicore_rpmsg_lite_MIMXRT685S_cm33)

include(middleware_multicore_rpmsg_lite_freertos)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_common)

include(driver_power)

include(driver_reset)

include(driver_dsp)

include(driver_inputmux)

include(middleware_multicore_rpmsg_lite_evkmimxrt685_freertos)

include(driver_mu)

include(driver_pca9420)

include(device_CMSIS)

include(utility_debug_console)

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

include(driver_lpc_gpio)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_MIMXRT685S_cm33)

include(middleware_freertos-kernel_extension)

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

