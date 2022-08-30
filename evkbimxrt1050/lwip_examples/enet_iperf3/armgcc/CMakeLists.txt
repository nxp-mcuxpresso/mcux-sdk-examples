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


project(lwip_enet_iperf3)

set(MCUX_SDK_PROJECT_NAME lwip_enet_iperf3.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../iperf_lwip.c"
"${ProjDirPath}/../iperf_api.h"
"${ProjDirPath}/../aws_wifi_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/board.c"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/board.h"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/clock_config.c"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/clock_config.h"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/dcd.c"
"${SdkRootDirPath}/core/boards/evkbimxrt1050/dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkbimxrt1050
    ${SdkRootDirPath}/examples/evkbimxrt1050/lwip_examples/enet_iperf3
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_cache_armv7_m7)

include(driver_mdio-enet)

include(driver_phy-device-ksz8081)

include(middleware_lwip_enet_ethernetif_MIMXRT1052)

include(middleware_lwip)

include(driver_phy-common)

include(driver_enet)

include(middleware_freertos-kernel_heap_4)

include(middleware_freertos-kernel_MIMXRT1052)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkbimxrt1050)

include(component_silicon_id)

include(CMSIS_Include_core_cm)

include(driver_mdio-common)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities_MIMXRT1052)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


