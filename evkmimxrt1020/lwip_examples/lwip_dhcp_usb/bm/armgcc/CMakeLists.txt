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


project(lwip_dhcp_usb_bm)

set(MCUX_SDK_PROJECT_NAME lwip_dhcp_usb_bm.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../lwip_dhcp_usb_bm.c"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../usb_host_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/clock_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1020/dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/evkmimxrt1020/lwip_examples/lwip_dhcp_usb/bm
    ${SdkRootDirPath}/core/boards/evkmimxrt1020
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_cache_armv7_m7)

include(middleware_usb_host_stack_MIMXRT1021)

include(component_osa)

include(middleware_usb_host_ehci)

include(middleware_usb_phy)

include(middleware_lwip_usb_ethernetif)

include(middleware_lwip)

include(middleware_usb_host_cdc)

include(middleware_usb_host_cdc_rndis)

include(driver_clock)

include(middleware_lwip_contrib_ping)

include(middleware_baremetal)

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

include(driver_xip_board_evkmimxrt1020)

include(component_silicon_id)

include(CMSIS_Include_core_cm)

include(middleware_usb_common_header)

include(middleware_usb_host_common_header)

include(utilities_misc_utilities_MIMXRT1021)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


