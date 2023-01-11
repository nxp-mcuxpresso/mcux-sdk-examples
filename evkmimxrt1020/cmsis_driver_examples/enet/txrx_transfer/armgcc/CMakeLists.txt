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


project(cmsis_enet_txrx_transfer)

set(MCUX_BUILD_TYPES sdram_debug sdram_release debug release flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME cmsis_enet_txrx_transfer.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../cmsis_enet_txrx_transfer.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../RTE_Device.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/clock_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmimxrt1020
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/devices/MIMXRT1021/drivers
    ${SdkRootDirPath}/core/drivers/enet
    ${SdkRootDirPath}/core/cmsis_drivers/enet
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/phy/device/phyksz8081
    ${SdkRootDirPath}/core/drivers/cache/armv7-m7
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/silicon_id
    ${SdkRootDirPath}/core/components/silicon_id/socs/rt10xx
    ${SdkRootDirPath}/core/devices/MIMXRT1021
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1021/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1020/xip
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/CMSIS/Driver/Include
    ${SdkRootDirPath}/core/drivers/edma
    ${SdkRootDirPath}/core/drivers/dmamux
    ${SdkRootDirPath}/core/components/phy
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_clock)

include(driver_enet)

include(driver_cmsis_enet)

include(driver_common)

include(driver_phy-device-ksz8081)

include(driver_cache_armv7_m7)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(component_silicon_id_MIMXRT1021)

include(component_silicon_id_rt10xx)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_lists)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1020)

include(CMSIS_Include_core_cm)

include(CMSIS_Driver_Include_Ethernet_MAC)

include(CMSIS_Driver_Include_Ethernet)

include(CMSIS_Driver_Include_Common)

include(CMSIS_Driver_Include_Ethernet_PHY)

include(driver_edma_MIMXRT1021)

include(driver_dmamux)

include(driver_phy-common_MIMXRT1021)

include(utilities_misc_utilities_MIMXRT1021)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


