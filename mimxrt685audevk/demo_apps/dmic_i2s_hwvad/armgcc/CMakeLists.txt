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


project(dmic_i2s_hwvad)

set(MCUX_SDK_PROJECT_NAME dmic_i2s_hwvad.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../dmic_i2s_hwvad.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.h"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/mimxrt685audevk/demo_apps/dmic_i2s_hwvad
    ${SdkRootDirPath}/core/boards/mimxrt685audevk
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_flexcomm_i2c)

include(component_codec_i2c_MIMXRT685S_cm33)

include(driver_common)

include(component_flexcomm_i2c_adapter)

include(utility_debug_console)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_sx1502)

include(utility_assert)

include(driver_cs42448)

include(driver_codec)

include(component_cs42448_adapter)

include(middleware_freertos-kernel_MIMXRT685S_cm33)

include(middleware_freertos-kernel_heap_4)

include(driver_dmic)

include(driver_flexcomm_i2s)

include(driver_flexcomm_usart)

include(driver_dmic_dma)

include(driver_lpc_dma)

include(driver_flexcomm_i2s_dma)

include(driver_inputmux)

include(driver_inputmux_connections)

include(driver_clock)

include(driver_power)

include(driver_reset)

include(device_CMSIS)

include(component_usart_adapter)

include(driver_flash_config_mimxrt685audevk)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_lpc_gpio)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities_MIMXRT685S_cm33)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

