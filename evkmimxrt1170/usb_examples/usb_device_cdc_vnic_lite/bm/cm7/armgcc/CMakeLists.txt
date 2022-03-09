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

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(dev_cdc_vnic_lite_bm_cm7)

set(MCUX_SDK_PROJECT_NAME dev_cdc_vnic_lite_bm_cm7.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../virtual_nic_enet_adapter.c"
"${ProjDirPath}/../virtual_nic_enet_adapter.h"
"${ProjDirPath}/../virtual_nic_enetif.c"
"${ProjDirPath}/../virtual_nic_enetif.h"
"${ProjDirPath}/../usb_device_descriptor.c"
"${ProjDirPath}/../usb_device_descriptor.h"
"${ProjDirPath}/../usb_device_cdc_acm.c"
"${ProjDirPath}/../usb_device_cdc_acm.h"
"${ProjDirPath}/../usb_device_cdc_rndis.c"
"${ProjDirPath}/../usb_device_cdc_rndis.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../virtual_nic.c"
"${ProjDirPath}/../virtual_nic.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_usb_device_ehci)

include(middleware_usb_phy)

include(driver_mdio-enet)

include(driver_phy-device-ksz8081)

include(driver_igpio)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(middleware_baremetal)

include(driver_enet)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_lists)

include(driver_lpuart)

include(device_startup_MIMXRT1176_cm7)

include(driver_iomuxc)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(middleware_usb_device_controller_driver_MIMXRT1176_cm7)

include(component_osa)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_armv7_m7)

include(middleware_usb_common_header)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(middleware_usb_device_common_header)

include(driver_mdio-common)

include(driver_memory)

include(driver_phy-common)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm7)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


