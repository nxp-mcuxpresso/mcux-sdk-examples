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


project(multi_peripherals_test)

set(MCUX_SDK_PROJECT_NAME multi_peripherals_test.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s16/board.c"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s16/board.h"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s16/clock_config.c"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s16/clock_config.h"
"${ProjDirPath}/../multi_peripherals_test.c"
"${ProjDirPath}/../accelerometer.c"
"${ProjDirPath}/../audio.c"
"${ProjDirPath}/../music.h"
"${ProjDirPath}/../led_blinky.c"
"${ProjDirPath}/../mouse.c"
"${ProjDirPath}/../mouse.h"
"${ProjDirPath}/../rtc.c"
"${ProjDirPath}/../usb_device_descriptor.c"
"${ProjDirPath}/../usb_device_descriptor.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../demo_config.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_class.c"
"${ProjDirPath}/../usb_device_class.h"
"${ProjDirPath}/../usb_device_hid.c"
"${ProjDirPath}/../usb_device_hid.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../core/boards/lpcxpresso55s16
    ${ProjDirPath}/../../../../../examples/lpcxpresso55s16/demo_apps/multi_peripherals_test
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(component_codec_i2c_LPC55S16)

include(driver_common)

include(component_flexcomm_i2c_adapter)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(middleware_usb_device_ip3511fs)

include(middleware_usb_device_controller_driver_LPC55S16)

include(component_osa)

include(middleware_usb_device_ip3511hs_LPC55S16)

include(driver_flexcomm_i2c)

include(driver_lpc_gpio)

include(driver_flexcomm_i2s)

include(driver_lpc_iocon)

include(driver_power)

include(driver_reset)

include(driver_lpc_rtc)

include(driver_flexcomm_usart)

include(driver_sysctl)

include(middleware_freertos-kernel_heap_4)

include(driver_codec)

include(component_wm8904_adapter)

include(middleware_usb_phy)

include(driver_clock)

include(device_CMSIS)

include(device_startup)

include(driver_flexcomm)

include(utility_assert)

include(utility_debug_console)

include(component_usart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_LPC55S16)

include(middleware_freertos-kernel_extension)

include(middleware_usb_device_common_header)

include(middleware_usb_common_header)

include(component_osa_free_rtos)

include(driver_wm8904)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


