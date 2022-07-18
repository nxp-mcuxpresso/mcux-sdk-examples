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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(cmsis_lpi2c_edma_b2b_transfer_slave_cm4)

set(MCUX_SDK_PROJECT_NAME cmsis_lpi2c_edma_b2b_transfer_slave_cm4.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../cmsis_lpi2c_edma_b2b_transfer_slave.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../RTE_Device.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/frdmk32l3a6/cmsis_driver_examples/lpi2c/edma_b2b_transfer/slave/cm4
    ${SdkRootDirPath}/core/boards/frdmk32l3a6
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

include(driver_port)

include(driver_clock)

include(driver_edma_K32L3A60_cm4)

include(driver_dmamux)

include(driver_lpi2c)

include(driver_lpi2c_edma)

include(driver_cmsis_lpi2c)

include(driver_common)

include(driver_msmc)

include(driver_lpuart)

include(driver_gpio)

include(component_lpuart_adapter)

include(component_lists)

include(device_CMSIS)

include(device_startup_K32L3A60_cm4)

include(CMSIS_Include_core_cm)

include(CMSIS_Driver_Include_I2C)

include(CMSIS_Driver_Include_Common)

include(utilities_misc_utilities_K32L3A60_cm4)

include(device_system_K32L3A60_cm4)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


