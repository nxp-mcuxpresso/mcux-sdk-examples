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


project(led_control_freertos)

set(MCUX_SDK_PROJECT_NAME led_control_freertos.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../led_control.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../hardware_init.c"
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../../../../../../core/boards/frdmk64f/board.c"
"${ProjDirPath}/../../../../../../core/boards/frdmk64f/board.h"
"${ProjDirPath}/../../../../../../core/boards/frdmk64f/clock_config.c"
"${ProjDirPath}/../../../../../../core/boards/frdmk64f/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../log/freertos
    ${ProjDirPath}/../../../../../../examples/frdmk64f/component_examples/led_control/freertos
    ${ProjDirPath}/../../../../../../core/boards/frdmk64f
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_ftm)

include(utility_shell)

include(driver_sysmpu)

include(component_led)

include(driver_common)

include(component_timer_manager)

include(component_gpio_adapter)

include(component_button)

include(component_lists)

include(component_ftm_adapter)

include(driver_gpio)

include(component_panic)

include(component_serial_manager)

include(component_serial_manager_uart)

include(component_osa_free_rtos)

include(middleware_freertos-kernel_heap_4)

include(driver_flash)

include(driver_clock)

include(utility_debug_console)

include(driver_port)

include(driver_uart)

include(driver_smc)

include(device_CMSIS)

include(component_uart_adapter)

include(device_startup)

include(utility_assert)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_MK64F12)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


