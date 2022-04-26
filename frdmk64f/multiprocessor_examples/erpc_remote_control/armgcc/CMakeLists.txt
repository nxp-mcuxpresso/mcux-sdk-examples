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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(erpc_remote_control)

set(MCUX_SDK_PROJECT_NAME erpc_remote_control.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../erpc_common/erpc_error_handler.h"
"${ProjDirPath}/../../erpc_common/erpc_error_handler.cpp"
"${ProjDirPath}/../remote_control.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../erpc_config.h"
"${ProjDirPath}/../service/erpc_remote_control.erpc"
"${ProjDirPath}/../service/erpc_remote_control.h"
"${ProjDirPath}/../service/erpc_remote_control_app_0.h"
"${ProjDirPath}/../service/erpc_remote_control_app_0_server.h"
"${ProjDirPath}/../service/erpc_remote_control_app_0_server.cpp"
"${ProjDirPath}/../service/erpc_remote_control_app_1.h"
"${ProjDirPath}/../service/erpc_remote_control_app_1_client.cpp"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../RTE_Device.h"
"${SdkRootDirPath}/core/boards/frdmk64f/board.c"
"${SdkRootDirPath}/core/boards/frdmk64f/board.h"
"${SdkRootDirPath}/core/boards/frdmk64f/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmk64f/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../erpc_common
    ${ProjDirPath}/../service
    ${SdkRootDirPath}/examples/frdmk64f/multiprocessor_examples/erpc_remote_control
    ${SdkRootDirPath}/core/boards/frdmk64f
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_uart)

include(driver_uart_edma)

include(driver_dac)

include(driver_fxos8700cq)

include(driver_cmsis_uart)

include(driver_gpio)

include(driver_port)

include(driver_i2c)

include(driver_adc16)

include(driver_dmamux)

include(middleware_multicore_erpc_eRPC_arbitrator)

include(middleware_multicore_erpc_eRPC_port_freertos)

include(middleware_multicore_erpc_eRPC_uart_cmsis_transport)

include(middleware_freertos-kernel_heap_4)

include(middleware_multicore_erpc_doc)

include(driver_flash)

include(driver_clock)

include(utility_debug_console)

include(driver_smc)

include(device_CMSIS)

include(component_uart_adapter)

include(component_serial_manager)

include(driver_common)

include(component_lists)

include(component_serial_manager_uart)

include(device_startup)

include(utility_assert)

include(CMSIS_Include_core_cm)

include(driver_edma_MK64F12)

include(CMSIS_Driver_Include_USART)

include(CMSIS_Driver_Include_Common)

include(middleware_multicore_erpc_common)

include(middleware_freertos-kernel_MK64F12)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE g)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE stdc++)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


