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


project(erpc_matrix_multiply_mu_rtos_cm4)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME erpc_matrix_multiply_mu_rtos_cm4.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_error_handler.h"
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_error_handler.cpp"
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply.erpc"
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply.h"
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply_client.cpp"
"${ProjDirPath}/../main_core0.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_matrix_multiply_mu_rtos/erpc_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_matrix_multiply/service
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_matrix_multiply_mu_rtos
    ${SdkRootDirPath}/core/boards/frdmk32l3a6
)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_matrix_multiply_mu_rtos/cm0plus/armgcc/debug)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_matrix_multiply_mu_rtos/cm0plus/armgcc/release)
endif(CMAKE_BUILD_TYPE STREQUAL release)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/middleware/multicore
    ${SdkRootDirPath}/core/drivers/gpio
    ${SdkRootDirPath}/core/drivers/port
    ${SdkRootDirPath}/core/devices/K32L3A60/drivers
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/msmc
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/devices/K32L3A60
    ${SdkRootDirPath}/core/drivers/mu
    ${SdkRootDirPath}/core/CMSIS/Core/Include
)

# include modules
include(middleware_multicore_mcmgr_k32l3a6)

include(driver_gpio)

include(driver_port)

include(driver_clock)

include(middleware_multicore_erpc_eRPC_client)

include(middleware_multicore_erpc_eRPC_mu_c_wrapper)

include(middleware_multicore_erpc_eRPC_mu_rtos_transport)

include(middleware_multicore_erpc_eRPC_port_freertos)

include(middleware_multicore_mcmgr)

include(middleware_freertos-kernel_heap_4)

include(middleware_multicore_erpc_doc)

include(utility_incbin)

include(middleware_freertos-kernel_K32L3A60_cm4)

include(driver_common)

include(driver_msmc)

include(driver_lpuart)

include(utility_assert)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(device_CMSIS)

include(device_startup_K32L3A60_cm4)

include(driver_mu)

include(CMSIS_Include_core_cm)

include(middleware_multicore_erpc_common)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities_K32L3A60_cm4)

include(device_system_K32L3A60_cm4)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


