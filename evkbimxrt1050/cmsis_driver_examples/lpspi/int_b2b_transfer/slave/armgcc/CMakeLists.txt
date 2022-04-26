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


project(cmsis_lpspi_int_b2b_transfer_slave)

set(MCUX_SDK_PROJECT_NAME cmsis_lpspi_int_b2b_transfer_slave.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../cmsis_lpspi_int_b2b_transfer_slave.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../RTE_Device.h"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/board.c"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/board.h"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/clock_config.c"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/clock_config.h"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/dcd.c"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/evkbimxrt1050/cmsis_driver_examples/lpspi/int_b2b_transfer/slave
    ${SdkRootDirPath}/core/boards/evkbimxrt1050
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_clock)

include(driver_lpuart)

include(driver_edma_MIMXRT1052)

include(driver_dmamux)

include(driver_lpspi)

include(driver_lpspi_edma)

include(driver_cmsis_lpspi)

include(driver_common)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_lists)

include(device_startup)

include(driver_iomuxc)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkbimxrt1050)

include(CMSIS_Include_core_cm)

include(CMSIS_Driver_Include_SPI)

include(CMSIS_Driver_Include_Common)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


