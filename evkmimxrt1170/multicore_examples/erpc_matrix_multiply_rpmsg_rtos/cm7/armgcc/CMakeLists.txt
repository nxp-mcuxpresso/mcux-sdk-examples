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


project(erpc_matrix_multiply_rpmsg_rtos_cm7)

set(MCUX_BUILD_TYPES flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME erpc_matrix_multiply_rpmsg_rtos_cm7.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_common/erpc_error_handler.h"
"${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_common/erpc_error_handler.cpp"
"${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply.erpc"
"${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply.h"
"${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply_client.cpp"
"${ProjDirPath}/../main_core0.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_matrix_multiply_rpmsg_rtos/rpmsg_config.h"
"${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_matrix_multiply_rpmsg_rtos/erpc_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/dcd.h"
"${ProjDirPath}/../evkmimxrt1170_connect_cm4_cm7side.jlinkscript"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmimxrt1170
    ${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_common
    ${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_common/erpc_matrix_multiply/service
    ${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_matrix_multiply_rpmsg_rtos
)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC ${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_matrix_multiply_rpmsg_rtos/cm4/armgcc/debug)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC ${SdkRootDirPath}/examples/evkmimxrt1170/multicore_examples/erpc_matrix_multiply_rpmsg_rtos/cm4/armgcc/release)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/middleware/multicore
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm7
    ${SdkRootDirPath}/core/drivers/mu
    ${SdkRootDirPath}/core/CMSIS/Core/Include
)

# include modules
include(middleware_multicore_mcmgr_imxrt1170)

include(middleware_multicore_rpmsg_lite_imxrt1170_freertos)

include(middleware_multicore_rpmsg_lite)

include(middleware_multicore_rpmsg_lite_freertos)

include(driver_clock)

include(middleware_multicore_erpc_eRPC_client)

include(middleware_multicore_erpc_eRPC_rpmsg_lite_rtos_master_c_wrapper)

include(middleware_multicore_erpc_eRPC_rpmsg_lite_rtos_transport)

include(middleware_multicore_erpc_eRPC_port_freertos)

include(middleware_multicore_mcmgr)

include(middleware_freertos-kernel_MIMXRT1176_cm7)

include(middleware_freertos-kernel_heap_4)

include(middleware_multicore_erpc_doc)

include(utility_incbin)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart_MIMXRT1176_cm7)

include(driver_lpuart)

include(device_startup_MIMXRT1176_cm7)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_armv7_m7)

include(driver_mu)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_extension)

include(middleware_multicore_erpc_common)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm7)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


