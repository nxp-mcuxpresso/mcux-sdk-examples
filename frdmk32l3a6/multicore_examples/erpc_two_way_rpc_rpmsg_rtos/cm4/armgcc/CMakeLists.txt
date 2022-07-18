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


project(erpc_two_way_rpc_rpmsg_rtos_cm4)

set(MCUX_SDK_PROJECT_NAME erpc_two_way_rpc_rpmsg_rtos_cm4.elf)

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
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../erpc_common
    ${ProjDirPath}/../../../erpc_common/erpc_two_way_rpc/service
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_two_way_rpc_rpmsg_rtos
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_two_way_rpc_rpmsg_rtos/cm4
    ${SdkRootDirPath}/core/boards/frdmk32l3a6
)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm0plus/armgcc/debug)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm0plus/armgcc/release)
endif(CMAKE_BUILD_TYPE STREQUAL release)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_multicore_mcmgr_k32l3a6)

include(middleware_multicore_rpmsg_lite_k32l3a6_freertos)

include(middleware_multicore_rpmsg_lite_K32L3A60_cm4)

include(middleware_multicore_rpmsg_lite_freertos)

include(driver_port)

include(driver_clock)

include(driver_mu)

include(middleware_multicore_erpc_eRPC_arbitrator)

include(middleware_multicore_erpc_eRPC_rpmsg_lite_rtos_master_c_wrapper)

include(middleware_multicore_erpc_eRPC_rpmsg_lite_rtos_transport)

include(middleware_multicore_erpc_eRPC_port_freertos)

include(middleware_freertos-kernel_K32L3A60_cm4)

include(middleware_multicore_mcmgr)

include(middleware_freertos-kernel_heap_4)

include(middleware_multicore_erpc_doc)

include(utility_incbin)

include(driver_common)

include(driver_msmc)

include(driver_lpuart)

include(driver_gpio)

include(utility_assert)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(device_CMSIS)

include(device_startup_K32L3A60_cm4)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_extension)

include(middleware_multicore_erpc_common)

include(utilities_misc_utilities_K32L3A60_cm4)

include(device_system_K32L3A60_cm4)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


