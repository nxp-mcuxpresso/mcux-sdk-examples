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


project(freertos_segger_sysview)

set(MCUX_SDK_PROJECT_NAME freertos_segger_sysview.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../SEGGER/SEGGER.h"
"${ProjDirPath}/../SEGGER/SEGGER_RTT.c"
"${ProjDirPath}/../SEGGER/SEGGER_RTT.h"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW.c"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW.h"
"${ProjDirPath}/../SEGGER/SEGGER_RTT_ASM_ARMv7M.S"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW_ConfDefaults.h"
"${ProjDirPath}/../SEGGER/SEGGER_SYSVIEW_Int.h"
"${ProjDirPath}/../Config/Global.h"
"${ProjDirPath}/../Config/SEGGER_RTT_Conf.h"
"${ProjDirPath}/../Config/SEGGER_SYSVIEW_Conf.h"
"${ProjDirPath}/../Sample/FreeRTOSV10/SEGGER_SYSVIEW_FreeRTOS.c"
"${ProjDirPath}/../Sample/FreeRTOSV10/SEGGER_SYSVIEW_FreeRTOS.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1010/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1010/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1010/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1010/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../SEGGER
    ${ProjDirPath}/../Config
    ${ProjDirPath}/../Sample/FreeRTOSV10
    ${SdkRootDirPath}/examples/evkmimxrt1010/rtos_examples/visualization/freertos_segger_sysview
    ${SdkRootDirPath}/core/boards/evkmimxrt1010
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_clock)

include(middleware_freertos-kernel_heap_4)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1010)

include(middleware_freertos-kernel_MIMXRT1011)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities_MIMXRT1011)

include(CMSIS_Include_core_cm)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


