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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(log_freertos_cm0plus)

set(MCUX_SDK_PROJECT_NAME log_freertos_cm0plus.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../log_main.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/frdmk32l3a6/component_examples/log/freertos/cm0plus
    ${SdkRootDirPath}/core/boards/frdmk32l3a6
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_gpio)

include(utility_shell)

include(component_lists)

include(component_panic)

include(component_serial_manager)

include(driver_common)

include(component_serial_manager_uart)

include(component_osa_free_rtos)

include(middleware_freertos-kernel_heap_4)

include(component_log)

include(component_log_backend_debugconsole)

include(component_log_backend_ringbuffer)

include(driver_clock)

include(driver_msmc)

include(driver_port)

include(driver_lpuart)

include(utility_assert)

include(utility_debug_console)

include(component_lpuart_adapter)

include(device_CMSIS)

include(device_startup_K32L3A60_cm0plus)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_K32L3A60_cm0plus)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities)

include(device_system_K32L3A60_cm0plus)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


