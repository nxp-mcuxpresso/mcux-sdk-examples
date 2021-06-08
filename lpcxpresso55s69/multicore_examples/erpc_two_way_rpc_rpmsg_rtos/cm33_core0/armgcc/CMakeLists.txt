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

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(erpc_two_way_rpc_rpmsg_rtos_cm33_core0)

set(MCUX_SDK_PROJECT_NAME erpc_two_way_rpc_rpmsg_rtos_cm33_core0.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../../erpc_common/erpc_error_handler.h"
"${ProjDirPath}/../../../erpc_common/erpc_error_handler.cpp"
"${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service/erpc_two_way_rpc.erpc"
"${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface.h"
"${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_client.cpp"
"${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface.h"
"${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_server.h"
"${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_server.cpp"
"${ProjDirPath}/../main_core0.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../../rpmsg_config.h"
"${ProjDirPath}/../../erpc_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../../../../../../core/boards/lpcxpresso55s69/board.c"
"${ProjDirPath}/../../../../../../core/boards/lpcxpresso55s69/board.h"
"${ProjDirPath}/../../../../../../core/boards/lpcxpresso55s69/clock_config.c"
"${ProjDirPath}/../../../../../../core/boards/lpcxpresso55s69/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/.. debug
    ${ProjDirPath}/../../../erpc_common
    ${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../../examples/lpcxpresso55s69/multicore_examples/erpc_two_way_rpc_rpmsg_rtos
    ${ProjDirPath}/../../../../../../examples/lpcxpresso55s69/multicore_examples/erpc_two_way_rpc_rpmsg_rtos/cm33_core0
    ${ProjDirPath}/../../../../../../core/boards/lpcxpresso55s69
)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm33_core1/armgcc/debug)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm33_core1/armgcc/release)
endif(CMAKE_BUILD_TYPE STREQUAL release)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_multicore_mcmgr_lpc55s69)

include(middleware_multicore_rpmsg_lite_lpcxpresso55s69_freertos)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_mailbox)

include(driver_lpc_iocon)

include(driver_lpc_gpio)

include(middleware_multicore_erpc_eRPC_arbitrator)

include(middleware_multicore_erpc_eRPC_rpmsg_lite_rtos_master_c_wrapper)

include(middleware_multicore_erpc_eRPC_rpmsg_lite_rtos_transport)

include(middleware_multicore_erpc_eRPC_port_freertos)

include(middleware_multicore_mcmgr)

include(middleware_freertos-kernel_LPC55S69_cm33_core0)

include(middleware_freertos-kernel_heap_3)

include(middleware_multicore_erpc_doc)

include(utility_incbin)

include(driver_common)

include(driver_power)

include(driver_reset)

include(driver_clock)

include(device_CMSIS)

include(device_startup_LPC55S69_cm33_core0)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(utility_assert)

include(utility_debug_console)

include(component_usart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(CMSIS_Include_core_cm33)

include(CMSIS_Include_common)

include(CMSIS_Include_dsp)

include(middleware_multicore_rpmsg_lite)

include(middleware_freertos-kernel_extension)

include(middleware_multicore_erpc_common)

include(utilities_misc_utilities)

include(device_system_LPC55S69_cm33_core0)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/devices/LPC55S69/gcc/libpower_hardabi.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/devices/LPC55S69/gcc/libpower_hardabi.a)
endif()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


