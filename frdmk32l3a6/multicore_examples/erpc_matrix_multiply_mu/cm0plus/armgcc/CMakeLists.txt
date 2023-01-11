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


project(erpc_matrix_multiply_mu_cm0plus)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME erpc_matrix_multiply_mu_cm0plus.elf)

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
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply_server.h"
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_matrix_multiply/service/erpc_matrix_multiply_server.cpp"
"${ProjDirPath}/../main_core1.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_matrix_multiply_mu/erpc_config.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/board.h"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmk32l3a6/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_common/erpc_matrix_multiply/service
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/frdmk32l3a6/multicore_examples/erpc_matrix_multiply_mu
    ${SdkRootDirPath}/core/boards/frdmk32l3a6
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/middleware/multicore
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/gpio
    ${SdkRootDirPath}/core/drivers/port
    ${SdkRootDirPath}/core/devices/K32L3A60/drivers
    ${SdkRootDirPath}/core
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/msmc
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/devices/K32L3A60
    ${SdkRootDirPath}/core/drivers/mu
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_multicore_mcmgr_k32l3a6)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_gpio)

include(driver_port)

include(driver_clock)

include(middleware_multicore_erpc_eRPC_server)

include(middleware_multicore_erpc_eRPC_mu_c_wrapper)

include(middleware_multicore_erpc_eRPC_mu_transport)

include(middleware_multicore_erpc_eRPC_port_stdlib)

include(middleware_multicore_mcmgr)

include(middleware_multicore_erpc_doc)

include(middleware_baremetal)

include(driver_common)

include(driver_msmc)

include(driver_lpuart)

include(component_lpuart_adapter)

include(component_lists)

include(device_CMSIS)

include(device_startup_K32L3A60_cm0plus)

include(driver_mu)

include(CMSIS_Include_core_cm)

include(middleware_multicore_erpc_common)

include(utilities_misc_utilities_K32L3A60_cm0plus)

include(device_system_K32L3A60_cm0plus)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/core1_image.bin)

