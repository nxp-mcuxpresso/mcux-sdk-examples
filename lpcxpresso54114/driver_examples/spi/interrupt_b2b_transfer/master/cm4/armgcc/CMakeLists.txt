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


project(spi_interrupt_b2b_transfer_master)

set(MCUX_SDK_PROJECT_NAME spi_interrupt_b2b_transfer_master.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../spi_interrupt_b2b_transfer_master.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../../../../../../../../core/boards/lpcxpresso54114/board.c"
"${ProjDirPath}/../../../../../../../../core/boards/lpcxpresso54114/board.h"
"${ProjDirPath}/../../../../../../../../core/boards/lpcxpresso54114/clock_config.c"
"${ProjDirPath}/../../../../../../../../core/boards/lpcxpresso54114/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../../../../core/boards/lpcxpresso54114
    ${ProjDirPath}/../../../../../../../../core/boards/lpcxpresso54114/driver_examples/spi/interrupt_b2b_transfer/master
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite_LPC54114_cm4)

include(driver_flexcomm_spi)

include(driver_common)

include(driver_clock)

include(driver_power)

include(driver_lpc_iocon)

include(driver_lpc_gpio)

include(driver_inputmux)

include(device_CMSIS_LPC54114_cm4)

include(CMSIS_Include_core_cm4)

include(device_startup_LPC54114_cm4)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(component_usart_adapter)

include(component_lists)

include(driver_reset)

include(driver_inputmux_connections)

include(utilities_misc_utilities)

include(CMSIS_Include_common)

include(CMSIS_Include_dsp)

include(device_system_LPC54114_cm4)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../../../core/devices/LPC54114/gcc/libpower_cm4_hardabi.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../../../core/devices/LPC54114/gcc/libpower_cm4_hardabi.a)
endif()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


