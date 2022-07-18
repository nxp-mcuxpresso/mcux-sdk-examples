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


project(rpmsg_lite_pingpong_cm33_core0)

set(MCUX_SDK_PROJECT_NAME rpmsg_lite_pingpong_cm33_core0.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_master.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../rpmsg_config.h"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/board.c"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/board.h"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/clock_config.c"
"${SdkRootDirPath}/core/boards/lpcxpresso55s69/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/lpcxpresso55s69/multicore_examples/rpmsg_lite_pingpong/cm33_core0
    ${SdkRootDirPath}/core/boards/lpcxpresso55s69
)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm33_core1/armgcc/debug)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm33_core1/armgcc/release)
endif(CMAKE_BUILD_TYPE STREQUAL release)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_multicore_mcmgr_lpc55s69)

include(middleware_multicore_rpmsg_lite_lpcxpresso55s69_bm)

include(middleware_multicore_rpmsg_lite_LPC55S69_cm33_core0)

include(middleware_multicore_rpmsg_lite_bm)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_mailbox)

include(driver_lpc_iocon)

include(middleware_baremetal)

include(middleware_multicore_mcmgr)

include(utility_incbin)

include(driver_common)

include(driver_power)

include(driver_reset)

include(driver_clock)

include(device_CMSIS)

include(device_startup_LPC55S69_cm33_core0)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(component_usart_adapter)

include(component_lists)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities_LPC55S69_cm33_core0)

include(device_system_LPC55S69_cm33_core0)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


