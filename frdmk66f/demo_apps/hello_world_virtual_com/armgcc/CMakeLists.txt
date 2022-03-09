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


project(hello_world_virtual_com)

set(MCUX_SDK_PROJECT_NAME hello_world_virtual_com.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../hello_world.c"
"${ProjDirPath}/../../../../../core/boards/frdmk66f/board.c"
"${ProjDirPath}/../../../../../core/boards/frdmk66f/board.h"
"${ProjDirPath}/../../../../../core/boards/frdmk66f/clock_config.c"
"${ProjDirPath}/../../../../../core/boards/frdmk66f/clock_config.h"
"${ProjDirPath}/../usb_device_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../examples/frdmk66f/demo_apps/hello_world_virtual_com
    ${ProjDirPath}/../../../../../core/boards/frdmk66f
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(component_serial_manager)

include(driver_common)

include(component_lists)

include(component_serial_manager_usb_cdc)

include(middleware_usb_device_controller_driver_MK66F18)

include(component_osa)

include(middleware_usb_device_ehci)

include(driver_sysmpu)

include(middleware_baremetal)

include(driver_clock)

include(driver_flash)

include(device_CMSIS)

include(utility_debug_console)

include(component_uart_adapter)

include(component_serial_manager_uart)

include(driver_uart)

include(device_startup)

include(driver_lpuart)

include(driver_port)

include(driver_smc)

include(driver_gpio)

include(utility_assert)

include(CMSIS_Include_core_cm)

include(middleware_usb_device_cdc_external)

include(middleware_usb_device_stack_external)

include(middleware_usb_phy)

include(middleware_usb_common_header)

include(middleware_usb_device_common_header)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


