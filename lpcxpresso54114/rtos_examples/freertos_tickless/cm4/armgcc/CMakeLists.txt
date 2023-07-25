# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

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


project(freertos_tickless)

set(MCUX_SDK_PROJECT_NAME freertos_tickless.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../low_power_tickless/cm4/fsl_tickless_rtc.c"
"${ProjDirPath}/../../low_power_tickless/cm4/fsl_tickless_rtc.h"
"${ProjDirPath}/../freertos_tickless.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/board.c"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/board.h"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/clock_config.c"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/.. debug
    ${ProjDirPath}/../../low_power_tickless/cm4
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/lpcxpresso54114
    ${SdkRootDirPath}/examples/lpcxpresso54114/rtos_examples/freertos_tickless
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_lpc_rtc)

include(driver_pint)

include(driver_clock)

include(driver_power)

include(middleware_freertos-kernel_heap_4)

include(driver_inputmux)

include(driver_common)

include(device_CMSIS)

include(CMSIS_Include_core_cm)

include(device_startup_LPC54114_cm4)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(driver_lpc_iocon)

include(driver_lpc_gpio)

include(utility_assert)

include(utility_debug_console)

include(component_usart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_reset)

include(middleware_freertos-kernel_LPC54114_cm4)

include(middleware_freertos-kernel_extension)

include(driver_inputmux_connections)

include(utilities_misc_utilities)

include(CMSIS_Include_core_cm)

include(CMSIS_DSP_Include)

include(device_system_LPC54114_cm4)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/devices/LPC54114/gcc/libpower_cm4_hardabi.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/devices/LPC54114/gcc/libpower_cm4_hardabi.a)
endif()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


