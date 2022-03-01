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


project(hello_world_virtual_com_cm7)

set(MCUX_SDK_PROJECT_NAME hello_world_virtual_com_cm7.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../hello_world.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/board.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/board.h"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/clock_config.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/clock_config.h"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/dcd.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/dcd.h"
"${ProjDirPath}/../evkmimxrt1160_connect_cm4_cm7side.jlinkscript"
"${ProjDirPath}/../usb_device_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../../examples/evkmimxrt1160/demo_apps/hello_world_virtual_com
    ${ProjDirPath}/../../../../../../examples/evkmimxrt1160/demo_apps/hello_world_virtual_com/cm7
    ${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(component_serial_manager)

include(driver_common)

include(component_lists)

include(component_serial_manager_usb_cdc)

include(middleware_usb_phy)

include(middleware_usb_device_controller_driver_MIMXRT1166_cm7)

include(component_osa)

include(middleware_usb_device_ehci)

include(middleware_baremetal)

include(driver_clock)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup_MIMXRT1166_cm7)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_armv7_m7)

include(CMSIS_Include_core_cm)

include(driver_anatop_ai)

include(middleware_usb_device_cdc_external)

include(middleware_usb_device_stack_external)

include(middleware_usb_common_header)

include(middleware_usb_device_common_header)

include(utilities_misc_utilities)

include(device_system_MIMXRT1166_cm7)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


