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


project(tflm_xaf_hifi4_cm33)

set(MCUX_BUILD_TYPES flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME tflm_xaf_hifi4_cm33.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/xaf/message.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/xaf/rpmsg_config.h"
"${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/tflm_xaf_hifi4/hifi4/binary/dsp_text_release.bin"
"${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/tflm_xaf_hifi4/hifi4/binary/dsp_data_release.bin"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../incbin.S"
"${ProjDirPath}/../dsp_support.c"
"${ProjDirPath}/../dsp_support.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/xaf/evkmimxrt685_hifi4/dsp_config.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_init.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/tflm_xaf_hifi4/hifi4/binary
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/xaf
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/xaf/evkmimxrt685_hifi4
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/drivers/i3c
    ${SdkRootDirPath}/core/components/codec/wm8904
    ${SdkRootDirPath}/core/drivers/mu
    ${SdkRootDirPath}/core
    ${SdkRootDirPath}/middleware/multicore
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/devices/MIMXRT685S
    ${SdkRootDirPath}/core/drivers/cache/cache64
    ${SdkRootDirPath}/core/devices/MIMXRT685S/drivers
    ${SdkRootDirPath}/core/boards/evkmimxrt685/flash_config
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/drivers/inputmux
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/drivers/lpc_iopctl
    ${SdkRootDirPath}/core/components/pca9420
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(component_codec_i2c_MIMXRT685S_cm33)

include(driver_common)

include(component_i3c_adapter)

include(driver_codec)

include(component_wm8904_adapter)

include(driver_flexcomm_i2c)

include(driver_i3c)

include(driver_wm8904)

include(driver_mu)

include(middleware_baremetal)

include(middleware_multicore_rpmsg_lite_evkmimxrt685_bm)

include(middleware_multicore_rpmsg_lite_MIMXRT685S_cm33)

include(middleware_multicore_rpmsg_lite_bm)

include(component_lists)

include(component_usart_adapter)

include(driver_flexcomm_usart)

include(device_CMSIS)

include(device_startup)

include(driver_cache_cache64)

include(driver_clock)

include(driver_dsp)

include(driver_flash_config_evkmimxrt685)

include(driver_flexcomm)

include(driver_flexspi)

include(driver_inputmux)

include(driver_lpc_gpio)

include(driver_lpc_iopctl)

include(driver_pca9420)

include(driver_power)

include(driver_reset)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities_MIMXRT685S_cm33)

include(driver_inputmux_connections)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


