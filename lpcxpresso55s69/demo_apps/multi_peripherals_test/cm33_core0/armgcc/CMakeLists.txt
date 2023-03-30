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


project(multi_peripherals_test)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME multi_peripherals_test.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/multi_peripherals_test.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/accelerometer.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/audio.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/music.h"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/led_blinky.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/mouse.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/mouse.h"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/rtc.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/sdcard.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/ffconf.h"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/usb_device_descriptor.c"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/usb_device_descriptor.h"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/usb_device_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/board.c"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/board.h"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/clock_config.c"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/clock_config.h"
"${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test/demo_config.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_class.c"
"${ProjDirPath}/../usb_device_class.h"
"${ProjDirPath}/../usb_device_hid.c"
"${ProjDirPath}/../usb_device_hid.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/lpcxpresso55s69/demo_apps/multi_peripherals_test
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/lpcxpresso55s69
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/middleware/usb
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/drivers/lpc_iocon
    ${SdkRootDirPath}/core/devices/LPC55S69/drivers
    ${SdkRootDirPath}/core/drivers/lpc_rtc
    ${SdkRootDirPath}/core/drivers/sysctl
    ${SdkRootDirPath}/core/drivers/sdif
    ${SdkRootDirPath}/middleware/sdmmc
    ${SdkRootDirPath}/middleware/fatfs
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/devices/LPC55S69
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/components/codec/wm8904
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(component_codec_i2c_LPC55S69_cm33_core0)

include(driver_common)

include(component_flexcomm_i2c_adapter)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(middleware_usb_device_ip3511fs)

include(middleware_usb_device_controller_driver_LPC55S69_cm33_core0)

include(component_osa)

include(middleware_usb_device_ip3511hs)

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

include(driver_sdif)

include(middleware_sdmmc_sd)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_sdif)

include(middleware_sdmmc_host_sdif_freertos)

include(middleware_fatfs)

include(middleware_fatfs_sd)

include(driver_codec)

include(component_wm8904_adapter)

include(middleware_usb_phy)

include(driver_clock)

include(device_CMSIS)

include(device_startup_LPC55S69_cm33_core0)

include(driver_flexcomm)

include(utility_assert)

include(utility_debug_console)

include(component_usart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart_LPC55S69_cm33_core0)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_LPC55S69_cm33_core0)

include(middleware_freertos-kernel_extension)

include(middleware_usb_device_common_header)

include(middleware_usb_common_header)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(driver_wm8904)

include(utilities_misc_utilities)

include(device_system_LPC55S69_cm33_core0)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


