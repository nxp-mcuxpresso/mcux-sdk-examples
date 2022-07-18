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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(cmsis_spi_dma_b2b_transfer_master)

set(MCUX_SDK_PROJECT_NAME cmsis_spi_dma_b2b_transfer_master.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../cmsis_spi_dma_b2b_transfer_master.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../RTE_Device.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/evkmimxrt595/cmsis_driver_examples/spi/dma_b2b_transfer/master
    ${SdkRootDirPath}/core/boards/evkmimxrt595
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_lpc_dma)

include(driver_flexcomm_spi)

include(driver_flexcomm_spi_dma)

include(driver_cmsis_flexcomm_spi)

include(driver_common)

include(driver_clock)

include(driver_power)

include(driver_inputmux)

include(driver_inputmux_connections)

include(device_CMSIS)

include(component_usart_adapter)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_lists)

include(driver_flexcomm_usart)

include(driver_flash_config_evkmimxrt595)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_lpc_gpio)

include(driver_reset)

include(CMSIS_Include_core_cm)

include(CMSIS_Driver_Include_SPI)

include(CMSIS_Driver_Include_Common)

include(utilities_misc_utilities_MIMXRT595S_cm33)

include(driver_iap)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

