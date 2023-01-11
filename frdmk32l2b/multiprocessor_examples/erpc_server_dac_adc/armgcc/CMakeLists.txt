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


project(erpc_server_dac_adc)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME erpc_server_dac_adc.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../fxos.c"
"${SdkRootDirPath}/examples/frdmk32l2b/multiprocessor_examples/erpc_common/erpc_error_handler.h"
"${SdkRootDirPath}/examples/frdmk32l2b/multiprocessor_examples/erpc_common/erpc_error_handler.cpp"
"${ProjDirPath}/../main_server.c"
"${ProjDirPath}/../erpc_config.h"
"${ProjDirPath}/../service/erpc_dac_adc.erpc"
"${ProjDirPath}/../service/erpc_dac_adc.h"
"${ProjDirPath}/../service/erpc_dac_adc_server.h"
"${ProjDirPath}/../service/erpc_dac_adc_server.cpp"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../RTE_Device.h"
"${SdkRootDirPath}/core/boards/frdmk32l2b/board.c"
"${SdkRootDirPath}/core/boards/frdmk32l2b/board.h"
"${SdkRootDirPath}/core/boards/frdmk32l2b/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmk32l2b/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/frdmk32l2b/multiprocessor_examples/erpc_common
    ${ProjDirPath}/..
    ${ProjDirPath}/../service
    ${SdkRootDirPath}/core/boards/frdmk32l2b
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/drivers/dac
    ${SdkRootDirPath}/core/components/fxos8700cq
    ${SdkRootDirPath}/core/cmsis_drivers/lpuart
    ${SdkRootDirPath}/core/drivers/gpio
    ${SdkRootDirPath}/core/drivers/port
    ${SdkRootDirPath}/core/drivers/i2c
    ${SdkRootDirPath}/core/drivers/adc16
    ${SdkRootDirPath}/core/drivers/dma
    ${SdkRootDirPath}/core/drivers/dmamux
    ${SdkRootDirPath}/middleware/multicore
    ${SdkRootDirPath}/core/devices/K32L2B31A/drivers
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/K32L2B31A
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/uart
    ${SdkRootDirPath}/core/drivers/smc
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/CMSIS/Driver/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_lpuart)

include(driver_lpuart_dma)

include(driver_dac)

include(driver_fxos8700cq)

include(driver_cmsis_lpuart)

include(driver_gpio)

include(driver_port)

include(driver_i2c)

include(driver_adc16)

include(driver_dma)

include(driver_dmamux)

include(middleware_multicore_erpc_eRPC_server)

include(middleware_multicore_erpc_eRPC_uart_cmsis_transport)

include(middleware_multicore_erpc_eRPC_port_stdlib)

include(middleware_multicore_erpc_doc)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(device_startup)

include(driver_uart)

include(driver_smc)

include(utility_assert)

include(CMSIS_Include_core_cm)

include(CMSIS_Driver_Include_USART)

include(CMSIS_Driver_Include_Common)

include(middleware_multicore_erpc_common)

include(utilities_misc_utilities_K32L2B31A)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lstdc++ -lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE g)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


