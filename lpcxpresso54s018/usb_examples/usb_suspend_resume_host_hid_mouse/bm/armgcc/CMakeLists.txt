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


project(host_suspend_resume_hid_mouse_bm)

set(MCUX_SDK_PROJECT_NAME host_suspend_resume_hid_mouse_bm.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../app.c"
"${ProjDirPath}/../host_mouse.c"
"${ProjDirPath}/../host_mouse.h"
"${ProjDirPath}/../usb_host_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_usb_host_ip3516hs_LPC54S018)

include(middleware_usb_host_ohci)

include(driver_gint)

include(component_lpc_gpio_adapter)

include(component_mrt_adapter)

include(driver_inputmux)

include(driver_inputmux_connections)

include(middleware_baremetal)

include(middleware_usb_host_hid)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(device_startup)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(driver_lpc_iocon)

include(driver_reset)

include(driver_power)

include(driver_emc)

include(utility_assert)

include(utility_debug_console)

include(component_usart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(middleware_usb_host_stack_LPC54S018)

include(component_osa)

include(middleware_usb_host_common_header)

include(CMSIS_Include_core_cm)

include(middleware_usb_common_header)

include(driver_pint)

include(driver_mrt)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/devices/LPC54S018/gcc/libpower_hardabi.a)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/devices/LPC54S018/gcc/libpower_hardabi.a)
endif(CMAKE_BUILD_TYPE STREQUAL release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


