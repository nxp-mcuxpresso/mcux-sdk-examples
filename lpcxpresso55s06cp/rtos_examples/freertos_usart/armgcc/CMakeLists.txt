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


project(freertos_usart)

set(MCUX_SDK_PROJECT_NAME freertos_usart.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../freertos_usart.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s06cp/board.c"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s06cp/board.h"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s06cp/clock_config.c"
"${ProjDirPath}/../../../../../core/boards/lpcxpresso55s06cp/clock_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../core/boards/lpcxpresso55s06cp
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_flexcomm_usart)

include(driver_flexcomm_usart_freertos)

include(driver_lpc_gpio)

include(middleware_freertos-kernel_heap_4)

include(driver_common)

include(driver_clock)

include(driver_power)

include(device_CMSIS)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iocon)

include(utility_assert)

include(utility_debug_console)

include(component_usart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(middleware_freertos-kernel_LPC5506CPXXXX)

include(middleware_freertos-kernel_extension)

include(driver_reset)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


