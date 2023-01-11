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


project(dsp_audio_demo_bm_cm33)

set(MCUX_BUILD_TYPES flash_debug flash_release debug release)

set(MCUX_SDK_PROJECT_NAME dsp_audio_demo_bm_cm33.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_cm33.c"
"${ProjDirPath}/../dsp_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../incbin.S"
"${ProjDirPath}/../dsp_support.c"
"${ProjDirPath}/../dsp_support.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.h"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.h"
"${SdkRootDirPath}/examples/mimxrt685audevk/dsp_examples/audio_demo_bm/dsp/binary/dsp_text_release.bin"
"${SdkRootDirPath}/examples/mimxrt685audevk/dsp_examples/audio_demo_bm/dsp/binary/dsp_data_release.bin"
"${SdkRootDirPath}/examples/mimxrt685audevk/dsp_examples/audio_demo_bm/dsp/binary/dsp_ncache_release.bin"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/mimxrt685audevk/dsp_examples/audio_demo_bm/dsp/binary
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/mimxrt685audevk
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/devices/MIMXRT685S/drivers
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/devices/MIMXRT685S/utilities
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/inputmux
    ${SdkRootDirPath}/core/components/pca9420
    ${SdkRootDirPath}/core/components/codec/cs42448
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/devices/MIMXRT685S
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/boards/mimxrt685audevk/flash_config
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/drivers/cache/cache64
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpc_iopctl
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_clock)

include(driver_flexcomm_i2c)

include(utility_shell)

include(driver_lpc_gpio)

include(driver_common)

include(driver_power)

include(driver_reset)

include(driver_dsp)

include(driver_inputmux)

include(driver_pca9420)

include(driver_cs42448)

include(driver_codec)

include(component_cs42448_adapter)

include(component_codec_i2c_MIMXRT685S_cm33)

include(component_flexcomm_i2c_adapter)

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

include(driver_inputmux_connections)

include(utilities_misc_utilities_MIMXRT685S_cm33)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

